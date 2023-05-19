"""Handles 'icpp build-native' """
# pylint: disable=unused-import, unused-argument
import sys
import subprocess
import shutil
import typer
from typing_extensions import Annotated

from icpp.__main__ import app
from icpp import config_default
from icpp.run_shell_cmd import run_shell_cmd

from icpp.decorators import requires_native_compiler, requires_pro
from icpp.options_build import to_compile_callback, option_to_compile_values_string


@app.command()
@requires_native_compiler()
@requires_pro("build-native")
def build_native(
    to_compile: Annotated[
        str,
        typer.Option(
            help=f"Files to compile {option_to_compile_values_string}.",
            callback=to_compile_callback,
        ),
    ] = "all",
) -> None:
    """Builds a native debug executable with your systems' Clang compiler.

    Reads 'icpp.toml' file in the current folder, and uses: \n
    (-) C++ & C files from [build-native] AND [build-wasm]\n
    (-) Only the compile flags of [build-native] \n
    (-) Only the link flags of [build-native]
    """
