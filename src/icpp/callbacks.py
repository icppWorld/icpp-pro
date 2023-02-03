"""Callback functions of our icpp typer app"""
from typing import Optional
import typer
from icpp.__main__ import app
from icpp import __version__, __version_wasi_sdk__


def version_callback(value: bool) -> None:
    """Prints version of icpp & wasi-sdk.

    When the user issues the command `icpp --version`, this callback is called.
    """
    if value:
        typer.echo(f"icpp version: {__version__}")
        typer.echo(f"wasi-sdk version: {__version_wasi_sdk__}")
        raise typer.Exit()


@app.callback()
# pylint: disable=unused-argument
def main_callback(
    # https://typer.tiangolo.com/tutorial/options/version/
    version: Optional[bool] = typer.Option(
        None,
        "--version",
        help="Prints version of icpp & wasi-sdk.",
        callback=version_callback,
        is_eager=True,
    ),
) -> None:
    """Modern C++ for the Internet Computer."""
