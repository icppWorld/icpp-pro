"""The marker a toolchain install writes last, so a later install can skip it.

It names what was installed. An install that failed partway wrote none, and an
install of other versions wrote another name: both are installed again.
"""

from pathlib import Path

import typer

MARKER_NAME = ".icpp-installed"


def is_complete(root: Path, installed: str) -> bool:
    """Returns True when an install into root of `installed` completed."""
    marker = root / MARKER_NAME
    try:
        return (
            marker.is_file() and marker.read_text(encoding="utf-8").strip() == installed
        )
    except (OSError, UnicodeDecodeError):
        return False


def skip(name: str, root: Path, installed: str, force: bool, files_ok: bool) -> bool:
    """Returns True, and says so, when the install can be skipped.

    Otherwise removes the marker first, so an install that fails is not taken
    as done, also when it cannot delete the old files.
    """
    if not force and files_ok and is_complete(root, installed):
        typer.echo(
            f"{name} is installed already in {root}\n(--force installs it again)"
        )
        return True
    (root / MARKER_NAME).unlink(missing_ok=True)
    return False


def mark_complete(root: Path, installed: str) -> None:
    """Records that the install into root of `installed` completed."""
    (root / MARKER_NAME).write_text(installed + "\n", encoding="utf-8")
