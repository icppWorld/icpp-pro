"""Handles 'icpp build-native' """
# pylint: disable=unused-import, unused-argument
import sys
import subprocess
import shutil
import typer

from icpp.__main__ import app
from icpp import config_default
from icpp.run_shell_cmd import run_shell_cmd

from icpp.decorators import requires_native_compiler, requires_pro


@app.command()
@requires_native_compiler()
@requires_pro()
def build_native() -> None:
    """Builds a native debug executable with your systems' Clang compiler.

    Reads 'icpp.toml' file in the current folder, and uses: \n
    (-) C++ & C files from [build-native] AND [build-wasm]\n
    (-) Only the compile flags of [build-native] \n
    (-) Only the link flags of [build-native]
    """
