"""Handles 'icpp init' """

import sys
import shutil
import typer
from icpp.__main__ import app
from icpp import config_default


@app.command()
def init() -> None:
    """Copies the greet project into a new subdirectory.

    See the Getting Started guide at https://github.com/icppWorld/icpp
    """
    icpp_init_path = config_default.ICPP_INIT_PATH
    local_init_path = config_default.LOCAL_INIT_PATH

    typer.echo("--")
    if local_init_path.exists():
        typer.echo(
            f"ERROR: Directory '{local_init_path}' already exists.\n"
            f"       Please remove it first."
        )
        sys.exit(1)

    shutil.copytree(icpp_init_path, local_init_path)
    typer.echo("Successfully created the `greet` project.")
