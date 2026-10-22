"""Handles 'icpp run-native'"""

import sys
from pathlib import Path
from typing import Annotated
import typer

from icpp.__main__ import app
from icpp.memory_guard import DEFAULT_MAX_GB, guard


@app.command()
def run_native(
    max_gb: Annotated[
        float,
        typer.Option(
            help="Memory limit in GiB: over it, the run is killed and fails.",
        ),
    ] = DEFAULT_MAX_GB,
) -> None:
    """Runs the native executable built by build-native, under a memory guard."""
    executable = Path("build-native") / "mockic.exe"
    if not executable.exists():
        typer.echo(f"ERROR: {executable} not found. Run 'icpp build-native' first.")
        sys.exit(1)
    sys.exit(guard([str(executable)], int(max_gb * 2**30)))
