"""A content-addressed cache for the icpp-candid library (__ic_candid__).

Every canister links the same library, so building it once per set of inputs
and reusing it saves about 100 compiles per build. A cached library is used
only when everything that goes into it is provably identical. The key is a
sha256 over:

- the sha256 of the preprocessed output (`-E`) of every source: whatever
  header the compiler would find, from wherever (search order, symlinks,
  `__has_include`, CPATH, a new header that shadows an old one), is in that
  text, so every way a header can change the build changes the key;
- the exact compile & archive commands, and the compiler driver's fully
  resolved command line (`-###`: config files, sysroot, flags set through the
  environment), for what codegen does with that text;
- the identity of each compiler (path, `--version`, size, mtime; on macOS of
  the real tool behind the xcrun shim);
- every file or folder a command names (resolved from the build folder): the
  content of a file, e.g. a profile or a plugin that is not in the preprocessed
  text, and the path of a folder;
- the icpp-pro & icpp-candid versions.

The key is computed again after a build, before the library is stored: when
anything changed during the build, nothing is stored.

Any doubt makes it a miss, and any error in this module prints a warning and
falls back to a normal build: the cache can never fail a build.

Entries live in ~/.icpp/cache/libraries/<target>/<key>/ and are published with
an atomic rename, so canisters built in parallel never see a partial entry.
Set ICPP_LIBRARY_CACHE=0 to turn the cache off; `rm -rf ~/.icpp/cache` clears it.

Trust: the cache is private to the user, like the compilers in ~/.icpp. Its
folders are created 0700 and its files 0600, and an entry is refused when any
level of it is a symlink, belongs to another user or is writable by others. A
restored library is hashed while it is copied into the canister folder, so a
file swapped after the validation is never linked.
"""

import concurrent.futures
import functools
import hashlib
import json
import os
import shlex
import shutil
import stat
import subprocess
import sys
import time
import uuid
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Dict, List, Optional, Sequence, Tuple

import typer

import icpp_candid
from icpp import __version__, config_default

# Bump to invalidate every entry, e.g. when what goes into the key changes
SCHEMA = 7

# The one library that is cached
CACHED_LIBRARY = "__ic_candid__"

MANIFEST = "manifest.json"

# Per target, the least recently used entries beyond these limits are removed.
# A native library is large (debug info: ~400 MiB), a wasm one is not (~15 MiB)
MAX_ENTRIES = 8
MAX_BYTES = 2 * 1024**3

# A temporary folder older than this is left over from a build that was killed
STALE_TMP_SECONDS = 3600


def enabled() -> bool:
    """False when ICPP_LIBRARY_CACHE is 0, false, no or off."""
    value = os.environ.get("ICPP_LIBRARY_CACHE", "")
    return value.strip().lower() not in ("0", "false", "no", "off")


def is_cached(lib_name: str) -> bool:
    """True for the library that is cached, when the cache is on."""
    return lib_name == CACHED_LIBRARY and enabled()


def _warn(message: str, after: str = "Building without it.") -> None:
    typer.echo(f"WARNING: library cache: {message}. {after}")


def _sha256_file(path: Path) -> str:
    with open(path, "rb") as f:
        return hashlib.file_digest(f, "sha256").hexdigest()


def _copy_hashed(src: Path, dst: Path) -> str:
    """Copies src to dst in one pass, and returns the sha256 of what was copied."""
    h = hashlib.sha256()
    with open(src, "rb") as fin, open(dst, "wb") as fout:
        for block in iter(lambda: fin.read(1 << 20), b""):
            h.update(block)
            fout.write(block)
    return h.hexdigest()


def _remove(path: Path) -> None:
    """Removes a folder, a file, or a symlink without following it."""
    try:
        if path.is_symlink() or not path.is_dir():
            path.unlink(missing_ok=True)
        else:
            shutil.rmtree(path, ignore_errors=True)
    except OSError:
        pass  # removed by a parallel build, or not ours to remove


def _target_root(target: str) -> Path:
    return config_default.ICPP_ROOT / "cache" / "libraries" / target


def _cache_levels(target: str) -> List[Path]:
    """The folders from ~/.icpp/cache down to the target folder."""
    cache = config_default.ICPP_ROOT / "cache"
    return [cache, cache / "libraries", cache / "libraries" / target]


def _untrusted(path: Path) -> Optional[str]:
    """None if path may be trusted, else why not: a symlink, owned by another
    user, or writable by group or others."""
    st = path.lstat()
    if stat.S_ISLNK(st.st_mode):
        return f"{path} is a symlink"
    if st.st_uid != os.getuid():
        return f"{path} belongs to another user"
    if st.st_mode & (stat.S_IWGRP | stat.S_IWOTH):
        return f"{path} is writable by others"
    return None


def _tighten(level: Path) -> None:
    """Makes a cache folder that we own, and that is not a symlink, 0700. Safe:
    a folder of another user or a symlink is left as it is (and refused), and an
    entry planted while a folder was open belongs to someone else (refused). It
    matters for a cache restored by `tar` (CI), which creates the folders above
    it with the umask, e.g. 0775."""
    if not os.path.lexists(level):
        return
    st = level.lstat()
    if stat.S_ISDIR(st.st_mode) and st.st_uid == os.getuid():
        os.chmod(level, 0o700)


def _private_root(target: str) -> Path:
    """Creates the cache folders as 0700, and checks they can be trusted."""
    config_default.ICPP_ROOT.mkdir(parents=True, exist_ok=True)
    for level in _cache_levels(target):
        if not level.exists():
            try:
                level.mkdir(mode=0o700)
            except FileExistsError:
                pass  # created by a parallel build
        _tighten(level)
        reason = _untrusted(level)
        if reason is not None:
            raise RuntimeError(reason)
    return _target_root(target)


def _split(command: str) -> List[str]:
    return shlex.split(command)


# Preprocessing runs in parallel; canisters may build in parallel too
MAX_WORKERS = max(1, (os.cpu_count() or 2) // 2)


@functools.lru_cache(maxsize=None)
def _version_output(path: str, size: int, mtime_ns: int) -> Tuple[str, int]:
    """`<path> --version`, run once per binary (size & mtime are in the cache key,
    so a replaced binary is asked again)."""
    _ = (size, mtime_ns)
    version = subprocess.run(
        [path, "--version"],
        capture_output=True,
        text=True,
        timeout=60,
        check=False,
    )
    return version.stdout + version.stderr, version.returncode


def _behind_xcrun(path: Path, program: str) -> Path:
    """On macOS, /usr/bin/clang, ar, ... are xcrun shims that stay the same when
    the Command Line Tools or Xcode are updated: the real tool behind them."""
    if sys.platform != "darwin" or path.parent != Path("/usr/bin"):
        return path
    result = subprocess.run(
        ["xcrun", "--find", Path(program).name],
        capture_output=True,
        text=True,
        timeout=60,
        check=False,
    )
    found = result.stdout.strip()
    return Path(found).resolve() if result.returncode == 0 and found else path


def _named_paths(commands: Dict[str, str], cwd: Path) -> List[Tuple[str, str]]:
    """Every file or folder a command names, resolved from cwd as the compiler
    resolves it: (path, sha256 of the content) for a file, (path, "folder") for
    a folder. A candidate is each argument, the part after its first `=`, and the
    part after a short flag (e.g. -Ipath). The compiler itself is in the
    identity instead."""
    named: Dict[str, str] = {}
    for command in commands.values():
        for token in _split(command)[1:]:
            candidates = [token]
            if "=" in token:
                candidates.append(token.split("=", 1)[1])
            if token.startswith("-") and not token.startswith("--"):
                candidates.append(token[2:])
            for candidate in candidates:
                if not candidate or candidate.startswith("-"):
                    continue
                path = os.path.normpath(os.path.join(cwd, candidate))
                if path in named:
                    continue
                if os.path.isfile(path):
                    named[path] = _sha256_file(Path(path))
                elif os.path.isdir(path):
                    named[path] = "folder"
    return sorted(named.items())


def _compiler_identity(command: str) -> Dict[str, Any]:
    """Who the compiler of a command is: path, `--version`, size and mtime."""
    program = _split(command)[0]
    found = shutil.which(program)
    if found is None:
        raise RuntimeError(f"compiler not found: {program}")
    real = _behind_xcrun(Path(found).resolve(), program)
    st = real.stat()
    output, returncode = _version_output(str(real), st.st_size, st.st_mtime_ns)
    return {
        "path": str(real),
        "size": st.st_size,
        "mtime_ns": st.st_mtime_ns,
        "version": output,
        "returncode": returncode,
    }


def _driver(command: str, language: str, cwd: Path) -> str:
    """The compiler driver's fully resolved commands for one compile (`-###`):
    the effect of config files, the sysroot and the flags set through the
    environment."""
    result = subprocess.run(
        f"{command} -### -c -x {language} - -o icpp_cache_probe.o",
        shell=True,
        input="",
        capture_output=True,
        text=True,
        timeout=120,
        check=False,
        cwd=cwd,
    )
    # The folder a compile runs in only goes into the debug info (e.g.
    # -fdebug-compilation-dir); the key holds it when the canister has own flags
    return result.stderr.replace(str(cwd.resolve()), "<cwd>")


def _preprocessed_sha256(command: str, source: str, cwd: Path) -> str:
    """The sha256 of the preprocessed output of one source: the text the
    compiler compiles, with every header it would include."""
    result = subprocess.run(
        f"{command} -E {source}",
        shell=True,
        capture_output=True,
        timeout=600,
        check=False,
        cwd=cwd,
    )
    if result.returncode != 0:
        raise RuntimeError(f"cannot preprocess {source}")
    return hashlib.sha256(result.stdout).hexdigest()


def _preprocessed(
    commands: Dict[str, str],
    cpp_sources: Sequence[str],
    c_sources: Sequence[str],
    cwd: Path,
) -> List[Tuple[str, str]]:
    """(source, sha256 of its preprocessed output), sorted, for every source,
    with the command the builder compiles it with."""
    jobs = [(s, commands["cpp"]) for s in cpp_sources]
    jobs += [(s, commands["c"]) for s in c_sources]
    with concurrent.futures.ThreadPoolExecutor(MAX_WORKERS) as executor:
        digests = executor.map(
            lambda job: _preprocessed_sha256(job[1], job[0], cwd), jobs
        )
        return sorted(zip((s for s, _ in jobs), digests))


def compute_key(  # pylint: disable=too-many-arguments, too-many-positional-arguments
    target: str,
    lib_name: str,
    commands: Dict[str, str],
    cpp_sources: Sequence[str],
    cwd: Optional[Path] = None,
    c_sources: Sequence[str] = (),
) -> str:
    """The sha256 of everything that goes into the library. cwd is the folder the
    compiles run in."""
    cwd = (cwd or Path.cwd()).resolve()
    data = {
        "schema": SCHEMA,
        "target": target,
        "lib_name": lib_name,
        "icpp_pro": __version__,
        "icpp_candid": icpp_candid.__version__,
        "commands": commands,
        "compilers": {
            name: _compiler_identity(command) for name, command in commands.items()
        },
        "drivers": {
            name: _driver(commands[name], language, cwd)
            for name, language in (("cpp", "c++"), ("c", "c"))
            if commands.get(name)
        },
        "named": _named_paths(commands, cwd),
        "sources": _preprocessed(commands, cpp_sources, c_sources, cwd),
    }
    return hashlib.sha256(json.dumps(data, sort_keys=True).encode()).hexdigest()


def _check(entry: Path, key: str, lib_file: str) -> Tuple[Optional[str], str]:
    """(None, the library's sha256) if the entry may be used, else (why not, "").
    The library itself is hashed by the caller, while it copies it."""
    manifest_path = entry / MANIFEST
    lib = entry / lib_file
    for path in (entry, manifest_path, lib):
        if not os.path.lexists(path):
            return f"{path.name} is missing", ""
        reason = _untrusted(path)
        if reason is not None:
            return reason, ""
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    if manifest.get("schema") != SCHEMA or manifest.get("key") != key:
        return "the manifest is for another key", ""
    if not lib.is_file():
        return f"{lib_file} is not a file", ""
    return None, str(manifest.get("lib_sha256"))


def _validate(entry: Path, key: str, lib_file: str) -> Tuple[Optional[str], str]:
    """_check, with any error as the reason."""
    try:
        return _check(entry, key, lib_file)
    except Exception as e:  # pylint: disable=broad-except
        return f"cannot read it: {e}", ""


def _intact(entry: Path, key: str, lib_file: str) -> bool:
    """True when an entry is valid, its library included."""
    reason, lib_sha256 = _validate(entry, key, lib_file)
    try:
        return reason is None and _sha256_file(entry / lib_file) == lib_sha256
    except OSError:
        return False


def restore(target: str, key: str, lib_file: str, build_path: Path) -> bool:
    """Copies a valid cached library into build_path. True on a hit."""
    entry = _target_root(target) / key
    copy = build_path / lib_file
    try:
        if not os.path.lexists(entry):
            return False
        reason: Optional[str] = None
        for level in _cache_levels(target):
            _tighten(level)
            reason = reason or _untrusted(level)
        lib_sha256 = ""
        if reason is None:
            reason, lib_sha256 = _validate(entry, key, lib_file)
        if reason is None:
            # Hashed while it is copied: what was copied is what is checked,
            # so a file swapped since the validation is never linked
            if _copy_hashed(entry / lib_file, copy) != lib_sha256:
                copy.unlink()
                reason = f"{lib_file} does not match its sha256"
        if reason is not None:
            typer.echo(f"Library cache: not using {entry}: {reason}")
            return False
    except Exception as e:  # pylint: disable=broad-except
        # A partial copy must not stay (the builder also removes the archive
        # before it archives, as `ar qc` appends to an existing one)
        try:
            copy.unlink(missing_ok=True)
        except OSError:
            pass
        _warn(f"cannot use {entry}: {e}")
        return False
    try:
        # Mark it as recently used, so pruning removes other entries first
        os.utime(entry / MANIFEST)
    except OSError:
        pass  # only the pruning order
    typer.echo(f"Using the cached library {lib_file} ({entry})")
    return True


def publish(
    target: str,
    key: str,
    lib_file: str,
    build_path: Path,
    commands: Dict[str, str],
) -> None:
    """Stores the library just built under key."""
    root = _target_root(target)
    tmp = root / f".tmp-{key}-{os.getpid()}-{uuid.uuid4().hex}"
    try:
        root = _private_root(target)
        tmp.mkdir(mode=0o700)
        lib_sha256 = _copy_hashed(build_path / lib_file, tmp / lib_file)
        os.chmod(tmp / lib_file, 0o600)
        manifest = {
            "schema": SCHEMA,
            "key": key,
            "lib_sha256": lib_sha256,
            "commands": commands,
            "icpp_pro": __version__,
            "icpp_candid": icpp_candid.__version__,
            "created": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
        }
        (tmp / MANIFEST).write_text(
            json.dumps(manifest, indent=1, sort_keys=True), encoding="utf-8"
        )
        os.chmod(tmp / MANIFEST, 0o600)
        os.chmod(tmp, 0o700)

        entry = root / key
        if os.path.lexists(entry):
            if _intact(entry, key, lib_file):
                # Another build published the same key first: keep it
                _remove(tmp)
                return
            stale = root / f".stale-{uuid.uuid4().hex}"
            try:
                os.rename(entry, stale)
                _remove(stale)
            except FileNotFoundError:
                pass  # a parallel build already moved it away
        try:
            os.rename(tmp, entry)
        except OSError:
            # Lost a race with another build of the same key
            _remove(tmp)
            return
        typer.echo(f"Stored the library in the cache: {entry}")
    except Exception as e:  # pylint: disable=broad-except
        _remove(tmp)
        _warn(f"cannot store the library: {e}", "The build itself is fine.")
        return
    try:
        prune(target, keep=key)
    except Exception as e:  # pylint: disable=broad-except
        typer.echo(f"WARNING: library cache: cannot prune {root}: {e}")


def _size(path: Path) -> int:
    total = 0
    for f in path.rglob("*"):
        try:
            if f.is_file():
                total += f.stat().st_size
        except OSError:
            pass  # removed by a parallel build
    return total


def prune(target: str, keep: str) -> None:
    """Removes the least recently used entries beyond MAX_ENTRIES / MAX_BYTES,
    never the entry `keep`, and temporary folders left by killed builds."""
    root = _target_root(target)
    now = time.time()
    entries = []
    for path in root.iterdir():
        try:
            if path.name.startswith("."):
                if now - path.lstat().st_mtime > STALE_TMP_SECONDS:
                    _remove(path)
                continue
            manifest = path / MANIFEST
            used = manifest.stat().st_mtime if manifest.exists() else 0.0
        except OSError:
            continue  # removed by a parallel build
        entries.append((used, path))
    # `keep` first, then the most recently used
    entries.sort(key=lambda e: (e[1].name == keep, e[0]), reverse=True)
    kept, total = 0, 0
    for _, path in entries:
        size = _size(path)
        if path.name == keep or (kept < MAX_ENTRIES and total + size <= MAX_BYTES):
            kept, total = kept + 1, total + size
            continue
        # Rename first, so a reader never sees a half-deleted entry
        gone = root / f".stale-{uuid.uuid4().hex}"
        try:
            os.rename(path, gone)
        except OSError:
            continue
        _remove(gone)


@dataclass
class LibraryBuild:  # pylint: disable=too-many-instance-attributes
    """One build of a library, with or without the cache."""

    target: str
    lib_name: str
    lib_file: str
    build_path: Path
    commands: Dict[str, str]
    cpp_sources: Sequence[str] = field(default_factory=list)
    c_sources: Sequence[str] = field(default_factory=list)
    key: Optional[str] = None
    hit: bool = False

    def publish(self) -> None:
        """Stores the library just built, unless the cache is not used, or an
        input changed during the build (the key is computed again)."""
        if self.key is None or self.hit:
            return
        try:
            key_after = compute_key(
                self.target,
                self.lib_name,
                self.commands,
                self.cpp_sources,
                self.build_path,
                self.c_sources,
            )
        except Exception as e:  # pylint: disable=broad-except
            _warn(f"cannot check the inputs again: {e}", "The build itself is fine.")
            return
        if key_after != self.key:
            typer.echo(
                "Library cache: not storing the library: "
                "an input changed during the build"
            )
            return
        publish(self.target, self.key, self.lib_file, self.build_path, self.commands)


def start(  # pylint: disable=too-many-arguments, too-many-positional-arguments
    target: str,
    lib_name: str,
    commands: Dict[str, str],
    sources: Tuple[Sequence[str], Sequence[str]],
    lib_file: str,
    build_path: Path,
) -> LibraryBuild:
    """Before building a library: restores the cached copy into build_path when
    there is a valid one (hit), else returns what publish() needs after the
    build. sources are the C++ and the C sources. build_path must be empty: a
    leftover object would end up in the archive. Never raises."""
    build = LibraryBuild(
        target, lib_name, lib_file, build_path, commands, sources[0], sources[1]
    )
    if not is_cached(lib_name):
        return build
    try:
        if any(build_path.iterdir()):
            _warn(f"{build_path} is not empty")
            return build
    except OSError as e:
        _warn(f"cannot read {build_path}: {e}")
        return build
    try:
        build.key = compute_key(
            target, lib_name, commands, sources[0], build_path, sources[1]
        )
    except Exception as e:  # pylint: disable=broad-except
        _warn(f"cannot compute the key: {e}")
        return build
    try:
        build.hit = restore(target, build.key, lib_file, build_path)
    except Exception as e:  # pylint: disable=broad-except
        # restore handles its own errors; this is the last line of defense, as
        # the cache must never fail a build
        _warn(f"cannot use the cache: {e}")
        build.key = None
    return build
