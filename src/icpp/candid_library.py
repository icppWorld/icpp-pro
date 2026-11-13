"""Reuses a saved icpp-candid library (__ic_candid__) from a folder that the
user or CI chooses, instead of building it again for every canister.

Turned on by `--candid-library-dir DIR`, or by ICPP_CANDID_LIBRARY_DIR. Off, the
build is exactly as before. With a folder, a build uses the library saved there
for its exact compile & archive commands, its list of source files, the target,
the host (OS and architecture), the `--version` of its compilers and the
icpp-pro & icpp-candid versions (the stamp), or builds the library and saves
it there.

Nothing checks the sources or headers: whoever sets the folder guarantees that
they do not change while the folder is in use. `make all-tests` uses a fresh
folder per run; CI keys its cached folder on the hashes of the sources.

Trust: a saved library is linked into a canister, so a planted one would be
code injection. The folder must be private to the user: everything is created
0700 / 0600, and the folder, its subfolders and the saved files are refused
when one of them is a symlink, belongs to another user, or is writable by
others (our own folders are made 0700 first, e.g. after a `tar` restore in CI).
The folder's parent must belong to us or to root, and be closed to others or
sticky (like /tmp), so that nobody else can swap the folder. On macOS, where an
ACL can let others write without any mode bit, and a volume can ignore
ownership (every file then seems ours), both are refused too.
"""

import ctypes
import ctypes.util
import errno
import functools
import hashlib
import json
import os
import platform
import shlex
import subprocess
import shutil
import stat
import uuid
from pathlib import Path
from typing import Any, Dict, Optional, Sequence, Set, Tuple

import typer

import icpp_candid
from icpp import __version__

STAMP = "stamp.json"


def folder(option: Optional[str]) -> Optional[Path]:
    """The folder from the option (which typer also fills from the environment
    variable), or None when reuse is off. Made absolute without resolving
    symlinks, so the trust checks see its real parent."""
    if not option:
        return None
    try:
        return Path(os.path.abspath(Path(option).expanduser()))
    except RuntimeError as e:  # ~user that does not exist
        _warn(f"{option}: {e}; not reusing the library")
        return None


def _compiler_version(command: str) -> str:
    """The `--version` of the compiler a command runs; "" when it has none."""
    try:
        result = subprocess.run(
            [shlex.split(command)[0], "--version"],
            capture_output=True,
            text=True,
            timeout=60,
            check=False,
        )
    except (OSError, ValueError, subprocess.SubprocessError):
        return ""
    return result.stdout if result.returncode == 0 else ""


def stamp(
    target: str, commands: Dict[str, str], sources: Sequence[str]
) -> Tuple[Dict[str, Any], str]:
    """What a saved library must match to be reused, and its sha256 (the name of
    the subfolder it is saved in). sources is the list of files it compiles:
    their names, not their content."""
    data = {
        "icpp_pro": __version__,
        "icpp_candid": icpp_candid.__version__,
        "target": target,
        "host": [platform.system(), platform.machine()],
        "compilers": {
            name: _compiler_version(commands[name])
            for name in ("cpp", "c")
            if name in commands
        },
        "commands": commands,
        "sources": sorted(str(s) for s in sources),
    }
    return data, hashlib.sha256(json.dumps(data, sort_keys=True).encode()).hexdigest()


def _copy_hashed(src: Path, dst: Path) -> str:
    """Copies src to dst in one pass, and returns the sha256 of what was copied."""
    h = hashlib.sha256()
    with open(src, "rb") as fin, open(dst, "wb") as fout:
        for block in iter(lambda: fin.read(1 << 20), b""):
            h.update(block)
            fout.write(block)
    return h.hexdigest()


def _warn(message: str) -> None:
    typer.echo(f"WARNING: candid library folder: {message}")


def _tighten(path: Path) -> None:
    """Makes a folder that we own, and that is not a symlink, 0700."""
    if os.path.lexists(path):
        st = path.lstat()
        if stat.S_ISDIR(st.st_mode) and st.st_uid == os.getuid():
            os.chmod(path, 0o700)


# macOS: an ACL entry that grants one of these lets others change a path
_ACL_WRITE_RIGHTS = {
    "write",
    "append",
    "add_file",
    "add_subdirectory",
    "delete",
    "delete_child",
    "writeattr",
    "writeextattr",
    "writesecurity",
    "chown",
}
_ACL_TYPE_EXTENDED = 0x00000100
_MNT_IGNORE_OWNERSHIP = 0x00200000


class _Statfs(ctypes.Structure):  # pylint: disable=too-few-public-methods
    """macOS struct statfs, the 64-bit inode variant."""

    _fields_ = [
        ("f_bsize", ctypes.c_uint32),
        ("f_iosize", ctypes.c_int32),
        ("f_blocks", ctypes.c_uint64),
        ("f_bfree", ctypes.c_uint64),
        ("f_bavail", ctypes.c_uint64),
        ("f_files", ctypes.c_uint64),
        ("f_ffree", ctypes.c_uint64),
        ("f_fsid", ctypes.c_int32 * 2),
        ("f_owner", ctypes.c_uint32),
        ("f_type", ctypes.c_uint32),
        ("f_flags", ctypes.c_uint32),
        ("f_fssubtype", ctypes.c_uint32),
        ("f_fstypename", ctypes.c_char * 16),
        ("f_mntonname", ctypes.c_char * 1024),
        ("f_mntfromname", ctypes.c_char * 1024),
        ("f_flags_ext", ctypes.c_uint32),
        ("f_reserved", ctypes.c_uint32 * 7),
    ]


@functools.lru_cache(maxsize=None)
def _darwin_libc() -> Optional[ctypes.CDLL]:
    """The C library on macOS, else None."""
    if platform.system() != "Darwin":
        return None
    libc = ctypes.CDLL(ctypes.util.find_library("c"), use_errno=True)
    libc.acl_get_link_np.restype = ctypes.c_void_p
    libc.acl_get_link_np.argtypes = [ctypes.c_char_p, ctypes.c_int]
    libc.acl_to_text.restype = ctypes.c_void_p
    libc.acl_to_text.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
    libc.acl_free.argtypes = [ctypes.c_void_p]
    return libc


@functools.lru_cache(maxsize=None)
def _darwin_statfs(libc: ctypes.CDLL) -> Any:
    """statfs with the 64-bit inode struct: x86_64 has both layouts, and names
    this one statfs$INODE64."""
    try:
        return libc["statfs$INODE64"]
    except AttributeError:
        return libc["statfs"]


def _acl_grants_write(text: str) -> bool:
    """Whether an ACL in acl_to_text form allows anyone a write right. Its
    entries read like group:<uuid>:everyone:12:deny:delete, and the allow or
    deny carries the entry's flags: allow,inherited,file_inherit."""
    for line in text.splitlines()[1:]:
        fields = line.split(":")
        if (
            len(fields) >= 6
            and fields[4].split(",")[0] == "allow"
            and _ACL_WRITE_RIGHTS & set(fields[5].split(","))
        ):
            return True
    return False


def _acl_untrusted(libc: ctypes.CDLL, path: Path) -> Optional[str]:
    """macOS: why the ACL of path lets others write to it, or None."""
    ctypes.set_errno(0)
    acl = libc.acl_get_link_np(os.fsencode(path), _ACL_TYPE_EXTENDED)
    if not acl:
        # ENOENT: the path has no ACL. Any other error: it could not be read
        if ctypes.get_errno() == errno.ENOENT:
            return None
        return f"{path}: cannot read its ACL"
    try:
        text = libc.acl_to_text(acl, None)
        if not text:
            return f"{path}: cannot read its ACL"
        try:
            acl_text = ctypes.string_at(text).decode(errors="replace")
        finally:
            libc.acl_free(text)
    finally:
        libc.acl_free(acl)
    if _acl_grants_write(acl_text):
        return f"{path} has an ACL that lets others write to it"
    return None


def _darwin_untrusted(path: Path) -> Optional[str]:
    """macOS: why others may write to path although its owner and mode say
    they cannot, or None."""
    libc = _darwin_libc()
    if libc is None:
        return None
    buf = _Statfs()
    if _darwin_statfs(libc)(os.fsencode(path), ctypes.byref(buf)) != 0:
        return f"{path}: cannot read its volume's flags"
    if buf.f_flags & _MNT_IGNORE_OWNERSHIP:
        return f"{path} is on a volume that ignores ownership"
    return _acl_untrusted(libc, path)


def _untrusted(path: Path) -> Optional[str]:
    """None if path may be trusted, else why not."""
    st = path.lstat()
    if stat.S_ISLNK(st.st_mode):
        return f"{path} is a symlink"
    if st.st_uid != os.getuid():
        return f"{path} belongs to another user"
    if st.st_mode & (stat.S_IWGRP | stat.S_IWOTH):
        return f"{path} is writable by its group or by others"
    return _darwin_untrusted(path)


def _safe_parent(root: Path) -> Optional[str]:
    """None when nobody else can swap the folder or anything above it: every
    existing folder from its parent up to /, also where a symlink leads, belongs
    to us or to root, and its group and others cannot write to it, unless it is
    sticky (like /tmp). Then a path checked stays the same path for the files
    read or written after the check."""
    todo = [root.parent]
    seen: Set[Path] = set()
    while todo:
        start = todo.pop()
        for level in (start, *start.parents):
            if level in seen or not os.path.lexists(level):
                continue
            seen.add(level)
            st = level.lstat()
            if st.st_uid not in (os.getuid(), 0):
                return f"{level} belongs to another user"
            if stat.S_ISLNK(st.st_mode):
                todo.append(level.parent / os.readlink(level))
            elif st.st_mode & (stat.S_IWGRP | stat.S_IWOTH) and not (
                st.st_mode & stat.S_ISVTX
            ):
                return f"{level} is writable by its group or by others"
            else:
                reason = _darwin_untrusted(level)
                if reason is not None:
                    return reason
    return None


def _trusted_folders(root: Path, target: str) -> Optional[str]:
    """None when the folder and its target subfolder exist and may be trusted
    (our own are made 0700 first), else why not."""
    reason = _safe_parent(root)
    if reason is not None:
        return reason
    for level in (root, root / target):
        if not os.path.lexists(level):
            return f"{level} does not exist"
        _tighten(level)
        reason = _untrusted(level)
        if reason is not None:
            return reason
    return None


def restore(root: Path, target: str, key: str, lib_file: str, build_path: Path) -> bool:
    """Copies the saved library for key into build_path. True when it did; a
    copy that does not match its sha256 is removed, and the library is built."""
    entry = root / target / key
    copy = build_path / lib_file
    try:
        if not os.path.lexists(entry):
            return False
        for reason in (
            _trusted_folders(root, target),
            *(_untrusted(p) for p in (entry, entry / STAMP, entry / lib_file)),
        ):
            if reason is not None:
                _warn(f"not using {entry}: {reason}; building it")
                return False
        saved = json.loads((entry / STAMP).read_text(encoding="utf-8"))
        if _copy_hashed(entry / lib_file, copy) != saved.get("lib_sha256"):
            copy.unlink()
            _warn(f"{entry / lib_file} does not match its sha256; building it")
            return False
    except Exception as e:  # pylint: disable=broad-except
        try:
            copy.unlink(missing_ok=True)
        except OSError:
            pass
        _warn(f"cannot use {entry}: {e}; building it")
        return False
    typer.echo(f"Using the saved icpp-candid library: {entry}")
    return True


def save(  # pylint: disable=too-many-arguments, too-many-positional-arguments
    root: Path,
    target: str,
    key: str,
    data: Dict[str, Any],
    lib_file: str,
    build_path: Path,
) -> None:
    """Saves the library just built under key. Atomic: written to a temporary
    folder, then renamed, so canisters built in parallel never see a partial
    copy. When another build saved the same key first, that copy is kept."""
    tmp = root / target / f".tmp-{uuid.uuid4().hex}"
    try:
        # Level by level: nothing is created inside a folder that is not trusted.
        # The existing folders above are checked first. Missing parents are ours,
        # so 0700: with the default permissions a umask of 002 makes them
        # group-writable, and the check would refuse the folder we just made.
        reason = _safe_parent(root)
        if reason is not None:
            raise RuntimeError(reason)
        for parent in reversed([root.parent, *root.parent.parents]):
            if not os.path.lexists(parent):
                parent.mkdir(mode=0o700, exist_ok=True)
        reason = _safe_parent(root)
        if reason is not None:
            raise RuntimeError(reason)
        for level in (root, root / target):
            # exist_ok: a parallel build may create it first; checked below
            level.mkdir(mode=0o700, exist_ok=True)
            _tighten(level)
            reason = _untrusted(level)
            if reason is not None:
                raise RuntimeError(reason)
        tmp.mkdir(mode=0o700)
        lib_sha256 = _copy_hashed(build_path / lib_file, tmp / lib_file)
        (tmp / STAMP).write_text(
            json.dumps(dict(data, lib_sha256=lib_sha256), indent=1, sort_keys=True),
            encoding="utf-8",
        )
        for file in (tmp / lib_file, tmp / STAMP):
            os.chmod(file, 0o600)
        os.chmod(tmp, 0o700)
        entry = root / target / key
        try:
            os.rename(tmp, entry)
        except OSError:
            if not entry.is_dir():
                raise
            shutil.rmtree(tmp, ignore_errors=True)  # saved by a parallel build
            return
        typer.echo(f"Saved the icpp-candid library: {entry}")
    except Exception as e:  # pylint: disable=broad-except
        shutil.rmtree(tmp, ignore_errors=True)
        _warn(f"cannot save the library in {root}: {e}; the build itself is fine")
