"""Callback functions of our icpp typer app"""

import platform
from typing import Optional
import typer
from icpp.__main__ import app
from icpp import (
    __version__,
    __version_wasi_sdk__,
    __version_rust__,
    __version_mingw64__,
    __version_ic_wasi_polyfill__,
    __version_wasi2ic__,
    __version_ic_wasm__,
)


def version_callback(value: bool) -> None:
    """Prints version of icpp & wasi-sdk.

    When the user issues the command `icpp --version`, this callback is called.
    """
    if value:
        typer.echo(f"icpp-pro         version: {__version__}")
        typer.echo(f"wasi-sdk         version: {__version_wasi_sdk__}")
        typer.echo(f"wasi2ic          version: {__version_wasi2ic__}")
        typer.echo(f"ic-wasi-polyfill version: {__version_ic_wasi_polyfill__}")
        typer.echo(f"rust             version: {__version_rust__}")
        if platform.win32_ver()[0]:
            typer.echo(f"mingw64          version: {__version_mingw64__}")
        typer.echo(f"ic-wasm          version: {__version_ic_wasm__}")
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
    """A development platform for C++ smart contracts of the Internet Computer"""
