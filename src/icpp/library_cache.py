"""A content-addressed cache for the icpp-candid library (__ic_candid__).

Every canister links the same library, so building it once per set of inputs
and reusing it saves about 100 compiles per build. A cached library is used
only when every input that went into it is provably identical:

- The key is a sha256 over the compile & archive commands, the content of every
  source, the identity of each compiler (path, `--version`, size, mtime), the
  environment variables a compiler reads, and the icpp-pro & icpp-candid
  versions.
- Each entry records every file the compiler read (from `-MD` dependency
  files: headers, also the system headers) with its sha256, and the file
  listing of every folder the compiler searches for headers and of every
  folder a dependency is in. Before an entry is used, all of them are checked
  again: a changed header, or a new one that would now be found first or
  satisfy a `__has_include`, makes it a miss.
- A library is not stored when any of its sources or headers changed while it
  was being built.

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

import functools
import hashlib
import json
import os
import shlex
import shutil
import stat
import subprocess
import time
import uuid
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple

import typer

import icpp_candid
from icpp import __version__, config_default

# Bump to invalidate every entry, e.g. when what goes into the key changes
SCHEMA = 2

# The one library that is cached
CACHED_LIBRARY = "__ic_candid__"

MANIFEST = "manifest.json"

# Per target, the least recently used entries beyond these limits are removed.
# A native library is large (debug info: ~400 MiB), a wasm one is not (~15 MiB)
MAX_ENTRIES = 8
MAX_BYTES = 2 * 1024**3

# A temporary folder older than this is left over from a build that was killed
STALE_TMP_SECONDS = 3600

# Environment variables that change what clang, clang++ or ar produce
COMPILER_ENVIRONMENT = (
    "CPATH",
    "C_INCLUDE_PATH",
    "CPLUS_INCLUDE_PATH",
    "OBJC_INCLUDE_PATH",
    "OBJCPLUS_INCLUDE_PATH",
    "SDKROOT",
    "DEVELOPER_DIR",
    "MACOSX_DEPLOYMENT_TARGET",
    "CCC_OVERRIDE_OPTIONS",
    "SOURCE_DATE_EPOCH",
    "ZERO_AR_DATE",
)

# The listing recorded for a folder that does not exist
MISSING = "missing"


def enabled() -> bool:
    """False when ICPP_LIBRARY_CACHE is 0, false, no or off."""
    value = os.environ.get("ICPP_LIBRARY_CACHE", "")
    return value.strip().lower() not in ("0", "false", "no", "off")


def is_cached(lib_name: str) -> bool:
    """True for the library that is cached, when the cache is on."""
    return lib_name == CACHED_LIBRARY and enabled()


def dep_flags(output_file: str) -> str:
    """The compile flags that write the dependency file of one object."""
    return f"-MD -MF {output_file}.d"


def _warn(message: str) -> None:
    typer.echo(f"WARNING: library cache: {message}. Building without it.")


def _sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for block in iter(lambda: f.read(1 << 20), b""):
            h.update(block)
    return h.hexdigest()


def _copy_hashed(src: Path, dst: Path) -> str:
    """Copies src to dst in one pass, and returns the sha256 of what was copied."""
    h = hashlib.sha256()
    with open(src, "rb") as fin, open(dst, "wb") as fout:
        for block in iter(lambda: fin.read(1 << 20), b""):
            h.update(block)
            fout.write(block)
    return h.hexdigest()


def _remove(path: Path) -> None:
    """Removes a folder, or a symlink without following it."""
    if path.is_symlink():
        path.unlink(missing_ok=True)
    else:
        shutil.rmtree(path, ignore_errors=True)


def _listing(folder: str) -> str:
    """The sha256 of the names in a folder, or MISSING."""
    if not os.path.isdir(folder):
        return MISSING
    names = "\n".join(sorted(os.listdir(folder)))
    return hashlib.sha256(names.encode()).hexdigest()


def _target_root(target: str) -> Path:
    return config_default.ICPP_ROOT / "cache" / "libraries" / target


def _cache_levels(target: str) -> List[Path]:
    """The folders from ~/.icpp/cache down to the target folder."""
    cache = config_default.ICPP_ROOT / "cache"
    return [cache, cache / "libraries", cache / "libraries" / target]


def _untrusted(path: Path) -> Optional[str]:
    """None if path may be trusted, else why not: a symlink, owned by another
    user, or writable by group or others. (No uids on Windows: only symlinks.)"""
    st = path.lstat()
    if stat.S_ISLNK(st.st_mode):
        return f"{path} is a symlink"
    if hasattr(os, "getuid"):
        if st.st_uid != os.getuid():
            return f"{path} belongs to another user"
        if st.st_mode & (stat.S_IWGRP | stat.S_IWOTH):
            return f"{path} is writable by others"
    return None


def _private_root(target: str) -> Path:
    """Creates the cache folders as 0700, and checks they can be trusted."""
    config_default.ICPP_ROOT.mkdir(parents=True, exist_ok=True)
    for level in _cache_levels(target):
        if not level.exists():
            try:
                level.mkdir(mode=0o700)
            except FileExistsError:
                pass  # created by a parallel build
        reason = _untrusted(level)
        if reason is not None:
            raise RuntimeError(reason)
        if hasattr(os, "getuid"):
            os.chmod(level, 0o700)
    return _target_root(target)


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


def _compiler_identity(command: str) -> Dict[str, Any]:
    """Who the compiler of a command is: path, `--version`, size and mtime."""
    program = shlex.split(command, posix=os.name != "nt")[0]
    found = shutil.which(program)
    if found is None:
        raise RuntimeError(f"compiler not found: {program}")
    real = Path(found).resolve()
    st = real.stat()
    output, returncode = _version_output(str(real), st.st_size, st.st_mtime_ns)
    return {
        "path": str(real),
        "size": st.st_size,
        "mtime_ns": st.st_mtime_ns,
        "version": output,
        "returncode": returncode,
    }


def compute_key(
    target: str, lib_name: str, commands: Dict[str, str], sources: List[str]
) -> str:
    """The sha256 of everything that goes into the library, but the headers."""
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
        "environment": {name: os.environ.get(name) for name in COMPILER_ENVIRONMENT},
        "sources": sorted(
            (str(Path(s).resolve()), _sha256_file(Path(s))) for s in sources
        ),
    }
    return hashlib.sha256(json.dumps(data, sort_keys=True).encode()).hexdigest()


def parse_dep_file(text: str, base: Path) -> List[Path]:
    """The prerequisites of a make rule written by `-MD`, as absolute paths."""
    text = text.replace("\\\r\n", " ").replace("\\\n", " ")
    _, sep, rest = text.partition(": ")
    if not sep:
        _, _, rest = text.partition(":\n")
    words: List[str] = []
    current = ""
    i = 0
    while i < len(rest):
        c = rest[i]
        if c == "\\" and i + 1 < len(rest) and rest[i + 1] in " #\\":
            current += rest[i + 1]
            i += 2
            continue
        if c == "$" and i + 1 < len(rest) and rest[i + 1] == "$":
            current += "$"
            i += 2
            continue
        if c.isspace():
            if current:
                words.append(current)
            current = ""
        else:
            current += c
        i += 1
    if current:
        words.append(current)
    return [
        (base / w).resolve() if not Path(w).is_absolute() else Path(w) for w in words
    ]


def parse_search_dirs(verbose_output: str) -> List[str]:
    """The header search folders from the output of `clang -E -v`."""
    folders: List[str] = []
    inside = False
    for line in verbose_output.splitlines():
        if "search starts here:" in line:
            inside = True
        elif line.startswith("End of search list."):
            inside = False
        elif inside and line.startswith(" "):
            folder = line.strip().removesuffix(" (framework directory)")
            folders.append(str(Path(folder).resolve()))
    return folders


def _search_dirs(commands: Dict[str, str]) -> List[str]:
    """The folders the compilers search for headers, as `-v` reports them."""
    folders: List[str] = []
    for name, language in (("cpp", "c++"), ("c", "c")):
        command = commands.get(name)
        if not command:
            continue
        result = subprocess.run(
            f"{command} -E -x {language} -v -",
            shell=True,
            input="",
            capture_output=True,
            text=True,
            timeout=120,
            check=True,
        )
        folders += parse_search_dirs(result.stderr)
    return folders


def _check(
    entry: Path, key: str, lib_file: str, hash_lib: bool
) -> Tuple[Optional[str], str]:
    """(None, the library's sha256) if the entry may be used, else (why not, "").
    With hash_lib False the library itself is not hashed: the caller hashes it
    while copying."""
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
    lib_sha256 = str(manifest.get("lib_sha256"))
    if not lib.is_file() or (hash_lib and _sha256_file(lib) != lib_sha256):
        return f"{lib_file} does not match its sha256", ""
    return _check_inputs(manifest), lib_sha256


def _check_inputs(manifest: Dict[str, Any]) -> Optional[str]:
    """None if every recorded dependency and folder listing is unchanged."""
    deps = manifest.get("deps")
    folders = manifest.get("folders")
    if not isinstance(deps, dict) or not deps or not isinstance(folders, dict):
        return "no inputs recorded"
    for path, sha in deps.items():
        p = Path(path)
        if not p.is_file():
            return f"a dependency is gone: {path}"
        if _sha256_file(p) != sha:
            return f"a dependency changed: {path}"
    for folder, listing in folders.items():
        if _listing(folder) != listing:
            return f"the files in a header folder changed: {folder}"
    return None


def _validate(
    entry: Path, key: str, lib_file: str, hash_lib: bool = True
) -> Tuple[Optional[str], str]:
    """_check, with any error as the reason."""
    try:
        return _check(entry, key, lib_file, hash_lib)
    except Exception as e:  # pylint: disable=broad-except
        return f"cannot read it: {e}", ""


def restore(target: str, key: str, lib_file: str, build_path: Path) -> bool:
    """Copies a valid cached library into build_path. True on a hit."""
    entry = _target_root(target) / key
    copy = build_path / lib_file
    try:
        if not os.path.lexists(entry):
            return False
        reason: Optional[str] = None
        for level in _cache_levels(target):
            reason = reason or _untrusted(level)
        lib_sha256 = ""
        if reason is None:
            reason, lib_sha256 = _validate(entry, key, lib_file, hash_lib=False)
        if reason is None:
            # Hashed while it is copied: what was copied is what is checked,
            # so a file swapped since the validation is never linked
            if _copy_hashed(entry / lib_file, copy) != lib_sha256:
                copy.unlink()
                reason = f"{lib_file} does not match its sha256"
        if reason is not None:
            typer.echo(f"Library cache: not using {entry}: {reason}")
            return False
        # Mark it as recently used, so pruning removes other entries first
        os.utime(entry / MANIFEST)
        typer.echo(f"Using the cached library {lib_file} ({entry})")
        return True
    except Exception as e:  # pylint: disable=broad-except
        # A partial copy must not stay: the build that follows appends to an
        # existing archive (`ar qc`) instead of replacing it
        copy.unlink(missing_ok=True)
        _warn(f"cannot use {entry}: {e}")
        return False


def _inputs(
    build_path: Path, commands: Dict[str, str], started_ns: Optional[int]
) -> Tuple[Dict[str, str], Dict[str, str]]:
    """The files the compiler read, with their sha256, and the listing of every
    header search folder and dependency folder. Raises ChangedDuringBuild when a
    file changed after the build started."""
    deps: Dict[str, str] = {}
    for dep_file in sorted(build_path.glob("*.d")):
        for path in parse_dep_file(dep_file.read_text(encoding="utf-8"), build_path):
            if str(path) in deps:
                continue
            if started_ns is not None and path.stat().st_mtime_ns > started_ns:
                raise ChangedDuringBuild(str(path))
            deps[str(path)] = _sha256_file(path)
    if not deps:
        raise RuntimeError("no dependency files were written")
    folders = set(_search_dirs(commands)) | {str(Path(d).parent) for d in deps}
    return deps, {folder: _listing(folder) for folder in sorted(folders)}


class ChangedDuringBuild(Exception):
    """A source or header changed while the library was being built."""


def publish(  # pylint: disable=too-many-arguments, too-many-positional-arguments
    target: str,
    key: str,
    lib_file: str,
    build_path: Path,
    commands: Dict[str, str],
    started_ns: Optional[int] = None,
) -> None:
    """Stores the library just built, with what the compiler read. With
    started_ns, nothing is stored when an input changed after that time."""
    root = _target_root(target)
    tmp = root / f".tmp-{key}-{os.getpid()}-{uuid.uuid4().hex}"
    try:
        deps, folders = _inputs(build_path, commands, started_ns)
        root = _private_root(target)
        tmp.mkdir(mode=0o700)
        lib_sha256 = _copy_hashed(build_path / lib_file, tmp / lib_file)
        os.chmod(tmp / lib_file, 0o600)
        manifest = {
            "schema": SCHEMA,
            "key": key,
            "lib_sha256": lib_sha256,
            "deps": deps,
            "folders": folders,
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
            if _validate(entry, key, lib_file)[0] is None:
                # Another build published the same key first: keep it
                _remove(tmp)
                return
            stale = root / f".stale-{uuid.uuid4().hex}"
            os.rename(entry, stale)
            _remove(stale)
        try:
            os.rename(tmp, entry)
        except OSError:
            # Lost a race with another build of the same key
            _remove(tmp)
            return
        typer.echo(f"Stored the library in the cache: {entry}")
    except ChangedDuringBuild as e:
        _remove(tmp)
        typer.echo(
            f"Library cache: not storing the library: {e} changed during the build"
        )
        return
    except Exception as e:  # pylint: disable=broad-except
        _remove(tmp)
        _warn(f"cannot store the library: {e}")
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
    total = 0
    for count, (_, path) in enumerate(entries, start=1):
        total += _size(path)
        if path.name == keep:
            continue
        if count > MAX_ENTRIES or total > MAX_BYTES:
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
    lib_file: str
    build_path: Path
    commands: Dict[str, str]
    key: Optional[str]
    hit: bool
    started_ns: int

    @property
    def with_deps(self) -> bool:
        """True when the compiles must write dependency files, to publish."""
        return self.key is not None and not self.hit

    def publish(self) -> None:
        """Stores the library just built, when the cache is used."""
        if self.with_deps and self.key is not None:
            publish(
                self.target,
                self.key,
                self.lib_file,
                self.build_path,
                self.commands,
                self.started_ns,
            )


def start(  # pylint: disable=too-many-arguments, too-many-positional-arguments
    target: str,
    lib_name: str,
    commands: Dict[str, str],
    sources: List[str],
    lib_file: str,
    build_path: Path,
    parallel: bool = True,
) -> LibraryBuild:
    """Before building a library: restores the cached copy into build_path when
    there is a valid one (hit), else tells whether to build for the cache.
    Only the parallel build writes the dependency files the cache needs."""
    started_ns = time.time_ns()
    build = LibraryBuild(
        target, lib_file, build_path, commands, None, False, started_ns
    )
    if not is_cached(lib_name) or not parallel:
        return build
    try:
        build.key = compute_key(target, lib_name, commands, sources)
    except Exception as e:  # pylint: disable=broad-except
        _warn(f"cannot compute the key: {e}")
        return build
    build.hit = restore(target, build.key, lib_file, build_path)
    return build
