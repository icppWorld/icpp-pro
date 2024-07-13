"""Handles 'icpp build-native' """

# pylint: disable=too-many-statements
import sys
import platform
import os
from pathlib import Path
import subprocess
import shutil
import concurrent.futures
import typer
from typing_extensions import Annotated

from icpp.__main__ import app
from icpp import config_default
from icpp.run_shell_cmd import run_shell_cmd

from icpp.decorators import requires_native_compiler
from icpp.options_build import (
    config_callback,
    to_compile_callback,
    option_to_compile_values_string,
)
from icpp.commands_build_library_native import build_library_native

OS_SYSTEM = platform.system()
OS_PROCESSOR = platform.processor()

# options are: "none", "multi-threading"
CONCURRENCY = "multi-threading"


@app.command()
@requires_native_compiler()
def build_native(
    config: Annotated[
        str,
        typer.Option(
            help="Name of configuration toml file.",
            callback=config_callback,
        ),
    ] = "icpp.toml",
    to_compile: Annotated[
        str,
        typer.Option(
            help=f"Files to compile {option_to_compile_values_string}.",
            callback=to_compile_callback,
        ),
    ] = "all",
) -> None:
    """Builds a native debug executable."""

    config_default.ICPP_TOML_PATH = Path(config)
    from icpp import icpp_toml  # pylint: disable = import-outside-toplevel

    # ----------------------------------------------------------------------
    # First build the libraries
    if to_compile != "mine-no-lib":
        build_library_native(config)

    # ----------------------------------------------------------------------
    typer.echo("----------------------------")
    typer.echo("Building the native executable...")

    # ----------------------------------------------------------------------
    build_path = icpp_toml.icpp_toml_path.parent / "build-native"
    typer.echo(f"Build folder: {build_path.resolve()}")

    if to_compile == "all":
        if build_path.exists():
            typer.echo("Deleting the build-native folder.")
            try:
                shutil.rmtree(build_path)
            except OSError as e:
                typer.echo(f"ERROR: During delete of {build_path}:\n {e.strerror}")

    if not build_path.exists():
        build_path.mkdir()

    cpp_files = icpp_toml.build_native["cpp_files"] + icpp_toml.build_wasm["cpp_files"]
    cpp_files_list = (
        icpp_toml.build_native["cpp_files_list"]
        + icpp_toml.build_wasm["cpp_files_list"]
    )
    cpp_include_flags = (
        icpp_toml.build_native["cpp_include_flags"]
        + icpp_toml.build_wasm["cpp_include_flags"]
    )
    cpp_compile_flags_s = icpp_toml.build_native["cpp_compile_flags_s"]
    cpp_link_flags_s = icpp_toml.build_native["cpp_link_flags_s"]

    c_files = icpp_toml.build_native["c_files"] + icpp_toml.build_wasm["c_files"]
    c_files_list = (
        icpp_toml.build_native["c_files_list"] + icpp_toml.build_wasm["c_files_list"]
    )
    c_include_flags = (
        icpp_toml.build_native["c_include_flags"]
        + icpp_toml.build_wasm["c_include_flags"]
    )
    c_compile_flags_s = icpp_toml.build_native["c_compile_flags_s"]

    cpp_compile_flags_defaults_s = config_default.NATIVE_CPPFLAGS
    cpp_link_flags_defaults_s = config_default.NATIVE_LDFLAGS
    c_compile_flags_defaults_s = config_default.NATIVE_CFLAGS

    if icpp_toml.build_native["overwrite_default_CPPFLAGS"]:
        cpp_compile_flags_defaults_s = icpp_toml.build_native[
            "cpp_compile_flags_defaults_s"
        ]
    if icpp_toml.build_native["overwrite_default_LDFLAGS"]:
        cpp_link_flags_defaults_s = icpp_toml.build_native["cpp_link_flags_defaults_s"]
    if icpp_toml.build_native["overwrite_default_CFLAGS"]:
        c_compile_flags_defaults_s = icpp_toml.build_native[
            "c_compile_flags_defaults_s"
        ]

    def cpp_compile_cmd_default() -> str:
        cmd = (
            f"{config_default.NATIVE_CPP} {config_default.NATIVE_CPP_REQUIRED_FLAGS} "
            f"{cpp_compile_flags_defaults_s} "
        )
        return cmd

    def c_compile_cmd_default() -> str:
        cmd = (
            f"{config_default.NATIVE_C} {config_default.NATIVE_C_REQUIRED_FLAGS} "
            f"{c_compile_flags_defaults_s} "
        )
        return cmd

    def cpp_compile_cmd_mine() -> str:
        cmd = (
            f"{cpp_compile_cmd_default()} "
            f"{cpp_include_flags} "
            f"{cpp_compile_flags_s} "
        )
        return cmd

    def c_compile_cmd_mine() -> str:
        cmd = f"{c_compile_cmd_default()} {c_include_flags} {c_compile_flags_s} "
        return cmd

    def cpp_compile_file_mine(file: str) -> None:
        # ensure unique *.o names, even if file has same name in a
        # pylint: disable=no-member
        output_file = (
            f"{file}".replace(f"{os.getcwd()}", "").replace("/", "_").replace("\\", "_")
        )
        cmd = f"{cpp_compile_cmd_mine()} -c {file} -o {output_file}.o"
        typer.echo(file)
        run_shell_cmd(cmd, cwd=build_path)

    def c_compile_file_mine(file: str) -> None:
        # ensure unique *.o names, even if file has same name in a subdirectory
        # pylint: disable=no-member
        output_file = (
            f"{file}".replace(f"{os.getcwd()}", "").replace("/", "_").replace("\\", "_")
        )
        cmd = f"{c_compile_cmd_mine()} -c {file} -o {output_file}.o"
        typer.echo(file)
        run_shell_cmd(cmd, cwd=build_path)

    try:
        # ----------------------------------------------------------------------
        # compile 'mine' C++ files, if we have any
        if len(cpp_files.strip()) > 0:
            if CONCURRENCY == "multi-threading":
                typer.echo("--")
                typer.echo("Compiling your C++ files:")
                typer.echo(f"Compile command: {cpp_compile_cmd_mine()}")
                with concurrent.futures.ThreadPoolExecutor() as executor:
                    executor.map(cpp_compile_file_mine, cpp_files_list)
            else:
                cmd = f"{cpp_compile_cmd_mine()} -c {cpp_files}"

                typer.echo("--")
                typer.echo("Compiling your C++ files with command:")
                typer.echo(cmd)
                run_shell_cmd(cmd, cwd=build_path)

        # ----------------------------------------------------------------------
        # compile 'mine' C files, if we have any
        if len(c_files.strip()) > 0:
            if CONCURRENCY == "multi-threading":
                typer.echo("--")
                typer.echo("Compiling your C files:")
                typer.echo(f"Compile command: {c_compile_cmd_mine()}")
                with concurrent.futures.ThreadPoolExecutor() as executor:
                    executor.map(c_compile_file_mine, c_files_list)
            else:
                cmd = f"{c_compile_cmd_mine()} -c {c_files}"

                typer.echo("--")
                typer.echo("Compiling your C files with command:")
                typer.echo(cmd)
                run_shell_cmd(cmd, cwd=build_path)

        # ----------------------------------------------------------------------
        # link it into a native executable
        executable = "mockic.exe"
        cmd = (
            f"{config_default.NATIVE_CPP} "
            f"{config_default.NATIVE_CPP_REQUIRED_FLAGS} "
            f"{cpp_link_flags_defaults_s} "
            f"{cpp_link_flags_s} "
            f"*.o "
        )

        # link the static libraries
        for library in icpp_toml.libraries:
            full_lib_name = f"{library['lib_name']}{config_default.WASM_AR_EXT}"
            full_lib_path = (
                icpp_toml.icpp_toml_path.parent
                / "build-library-native"
                / library["lib_name"]
                / full_lib_name
            ).resolve()
            cmd += f"{full_lib_path} "

        cmd += f"-o {executable} "

        typer.echo("--")
        typer.echo("Linking it into a native executable with command:")
        typer.echo(cmd)
        run_shell_cmd(cmd, cwd=build_path)

    except subprocess.CalledProcessError as e:
        sys.exit(e.returncode)
    except Exception as e:  # pylint: disable=broad-except
        typer.echo(
            f"ERROR: something went wrong in module 'commands_build_native.py' \n "
            f"Exception is: \n{e}"
        )
        sys.exit(1)

    # ----------------------------------------------------------------------
    # On Windows, we need to copy the runtime DLLs to the build folder
    # We do it this way, so the user does not have to set a system wide PATH
    if OS_SYSTEM == "Windows":
        typer.echo("--")
        typer.echo("Copying the MinGW-w64 runtime DLLs to 'build-native' folder")
        for dll_file in config_default.MINGW64_BIN.glob("*.dll"):
            shutil.copy(dll_file, build_path)

    # ----------------------------------------------------------------------
    # All done
    typer.echo("--")
    typer.echo("All done building the native debug executable:")
    typer.echo(f"{build_path.resolve()}/{executable}")
    try:
        typer.echo("(✔)You can run it from the command line")
        typer.echo("(✔)You can debug it with VS Code + CodeLLDB")
    except UnicodeEncodeError:
        typer.echo("(-)You can run it from the command line")
        typer.echo("(-)You can debug it with VS Code + CodeLLDB")

    # ----------------------------------------------------------------------
    typer.echo("-----")
