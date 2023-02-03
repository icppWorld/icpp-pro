"""Handles 'icpp get-...' """

import typer
from icpp.__main__ import app
from icpp import config_default


@app.command()
def get_icpp_path() -> None:
    """Prints path to icpp folder.

    For use in your build commands.
    """
    typer.echo(config_default.ICPP_DIR)


@app.command()
def get_api_c_and_cpp_files() -> None:
    """Lists all C & C++ files of the IC API.

    For use in your build commands.
    """
    typer.echo(config_default.IC_C_FILES + config_default.IC_CPP_FILES)


@app.command()
def get_api_header_files() -> None:
    """Lists all header files of the IC API.

    For use in your build commands.
    """
    typer.echo(config_default.IC_HEADER_FILES)


@app.command()
def get_mock_ic_cpp_files() -> None:
    """Lists all C++ files of the native debug Mock IC.

    For use in your native debug build commands.
    """
    typer.echo(config_default.MOCKIC_CPP_FILES)


@app.command()
def get_mock_ic_header_files() -> None:
    """Lists all header files of the native debug Mock IC.

    For use in your native debug build commands.
    """
    typer.echo(config_default.MOCKIC_HEADER_FILES)
