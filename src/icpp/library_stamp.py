"""The icpp version that built each library of a project.

`--to-compile mine-no-lib` links the libraries that are already in the
build-library folder. After an upgrade of icpp they hold the objects that the
previous version compiled, icpp's own (`__ic_candid__`) among them, and the
wasm or the native executable would link the old code: with no error at all
when no declaration changed. Once a library is built, its folder gets a stamp
with this version. Every build of a library, by any icpp version, starts by
deleting its folder, so mine-no-lib builds the libraries again when a library
or its stamp is missing, or a stamp is unreadable or names another version.
"""

from pathlib import Path
from typing import Any, Dict, List, Optional

import typer

from icpp import __version__

# A library folder is named after its lib_name, which can't start with '.'
STAMP_NAME = ".icpp-version"


def after_build(
    build_root: Path, libraries: List[Dict[Any, Any]], lib_name: Optional[str]
) -> None:
    """Stamps the libraries that a build just made: all of them, or the one
    named lib_name. Call it only when the build succeeded."""
    for library in libraries:
        if lib_name in (None, library["lib_name"]):
            stamp = build_root / library["lib_name"] / STAMP_NAME
            try:
                stamp.write_text(__version__, encoding="utf-8")
            except OSError as e:
                # The library is built: mine-no-lib builds it again, that's all
                typer.echo(f"WARNING: cannot write {stamp}: {e}")


def must_rebuild(
    build_root: Path, libraries: List[Dict[Any, Any]], ar_ext: str
) -> bool:
    """Whether mine-no-lib must build the libraries again. Says why, if so."""
    reason = _reason_to_rebuild(build_root, libraries, ar_ext)
    if reason is not None:
        typer.echo(f"{reason}: building the libraries.")
    return reason is not None


def _reason_to_rebuild(
    build_root: Path, libraries: List[Dict[Any, Any]], ar_ext: str
) -> Optional[str]:
    """Why the libraries in build_root can't be reused, or None if they can."""
    # The build refuses a symlink, as it would refuse to build into it
    if build_root.is_symlink():
        return f"The folder {build_root} is a symlink"
    for library in libraries:
        folder = build_root / library["lib_name"]
        archive = folder / f"{library['lib_name']}{ar_ext}"
        for path in (folder, archive, folder / STAMP_NAME):
            if path.is_symlink():
                return f"{path} is a symlink"
        if not archive.is_file():
            return f"The library {archive} is missing"
        version = _version(folder / STAMP_NAME)
        if version is None:
            return f"The library {archive} has no readable icpp version stamp"
        if version != __version__:
            return (
                f"The library {archive} was built by icpp {version}, "
                f"and this is icpp {__version__}"
            )
    return None


def _version(stamp: Path) -> Optional[str]:
    """The version in the stamp, or None when there is none or it is unreadable."""
    try:
        return stamp.read_text(encoding="utf-8").strip() or None
    except (OSError, UnicodeDecodeError):
        return None
