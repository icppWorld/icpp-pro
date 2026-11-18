"""The icpp version that built the libraries of a project.

`--to-compile mine-no-lib` links the libraries that are already in the
build-library folder. After an upgrade of icpp they hold the objects that the
previous version compiled, icpp's own (`__ic_candid__`) among them, and the
wasm or the native executable would link the old code: with no error at all
when no declaration changed. A build of all the libraries writes a stamp with
its version, and mine-no-lib builds them again when the stamp is missing,
unreadable or names another version, or when a library is missing or newer
than the stamp: another version built it, or its build failed partway. A
build of one library that succeeds refreshes a stamp of this version, unless
another library is newer than it, and drops it otherwise.
"""

from pathlib import Path
from typing import Any, Dict, List, Optional

import typer

from icpp import __version__

# A library folder is named after its lib_name, which can't start with '.'
STAMP_NAME = ".icpp-version"


def after_build(
    build_root: Path,
    libraries: List[Dict[Any, Any]],
    lib_name: Optional[str],
    ar_ext: str,
) -> None:
    """After a library build that succeeded: of all the libraries, or of the one
    named lib_name. A name of no library built nothing, and vouches for nothing."""
    if lib_name is None:
        _write(build_root)
    elif any(library["lib_name"] == lib_name for library in libraries):
        _refresh(build_root, build_root / lib_name / f"{lib_name}{ar_ext}")


def _write(build_root: Path) -> None:
    """Records that this icpp version built all the libraries in build_root.

    The full build deletes build_root first, so a build that fails partway
    leaves no stamp.
    """
    (build_root / STAMP_NAME).write_text(__version__, encoding="utf-8")


def _refresh(build_root: Path, built: Path) -> None:
    """After the library archive `built` was built: a stamp of this version
    still holds, and is made newer than it. It no longer holds when it names
    another version, since the libraries are then a mix of two versions, nor
    when another library is newer than it: making the stamp newer would hide
    that library from mine-no-lib."""
    stamp = build_root / STAMP_NAME
    others = [
        a for a in build_root.glob(f"*/*{built.suffix}") if a != built and a.is_file()
    ]
    if _version(stamp) == __version__ and _newer_than(stamp, others) is None:
        stamp.touch()
    else:
        stamp.unlink(missing_ok=True)


def _version(stamp: Path) -> Optional[str]:
    """The version in the stamp, or None when there is none or it is unreadable."""
    try:
        return stamp.read_text(encoding="utf-8").strip()
    except (OSError, UnicodeDecodeError):
        return None


def must_rebuild(
    build_root: Path, libraries: List[Dict[Any, Any]], ar_ext: str
) -> bool:
    """Whether mine-no-lib must build the libraries again. Says why, if so."""
    reason = _reason_to_rebuild(
        build_root,
        [
            build_root / lib["lib_name"] / f"{lib['lib_name']}{ar_ext}"
            for lib in libraries
        ],
    )
    if reason is not None:
        typer.echo(f"{reason}: building the libraries.")
    return reason is not None


def _reason_to_rebuild(build_root: Path, archives: List[Path]) -> Optional[str]:
    """Why the libraries in build_root can't be reused, or None if they can."""
    stamp = build_root / STAMP_NAME
    version = _version(stamp)
    if version is None:
        return f"The libraries in {build_root} have no readable icpp version stamp"
    if version != __version__:
        return (
            f"The libraries in {build_root} were built by icpp {version}, "
            f"and this is icpp {__version__}"
        )
    for archive in archives:
        if not archive.is_file():
            return f"The library {archive} is missing"
    newer = _newer_than(stamp, archives)
    if newer is not None:
        return f"The library {newer} is newer than the icpp version stamp"
    return None


def _newer_than(stamp: Path, archives: List[Path]) -> Optional[Path]:
    """The first of the archives that is newer than the stamp, if any."""
    stamped = stamp.stat().st_mtime
    return next((a for a in archives if a.stat().st_mtime > stamped), None)
