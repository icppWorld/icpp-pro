"""The icpp version that built the libraries of a project.

`--to-compile mine-no-lib` links the libraries that are already in the
build-library folder. After an upgrade of icpp they hold the objects that the
previous version compiled, icpp's own (`__ic_candid__`) among them, and the
wasm or the native executable would link the old code: with no error at all
when no declaration changed. A build of all the libraries writes a stamp with
its version, and mine-no-lib builds them again when the stamp is missing,
unreadable or names another version, or when a library is missing or newer
than the stamp: another version built it, or its build failed partway. A
build of one library that succeeds refreshes a stamp of this version and drops
one of another version.
"""

from pathlib import Path
from typing import Any, Dict, List, Optional

import typer

from icpp import __version__

# A library folder is named after its lib_name, which can't start with '.'
STAMP_NAME = ".icpp-version"


def write(build_root: Path) -> None:
    """Records that this icpp version built all the libraries in build_root.

    Call it only when every library was built: the full build deletes
    build_root first, so a build that fails partway leaves no stamp.
    """
    (build_root / STAMP_NAME).write_text(__version__, encoding="utf-8")


def refresh(build_root: Path) -> None:
    """After one library was built: a stamp of this version still holds, and is
    made newer than the library. A stamp of another version no longer holds,
    since the libraries are then a mix of two versions."""
    stamp = build_root / STAMP_NAME
    if _version(stamp) == __version__:
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
    stamped = stamp.stat().st_mtime
    for archive in archives:
        if not archive.is_file():
            return f"The library {archive} is missing"
        if archive.stat().st_mtime > stamped:
            return f"The library {archive} is newer than the icpp version stamp"
    return None
