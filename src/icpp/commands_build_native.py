"""Handles 'icpp build-native' """
# pylint: disable=too-many-statements
import sys
import subprocess
import shutil
import concurrent.futures
import typer
from typing_extensions import Annotated

from icpp.__main__ import app
from icpp import config_default
from icpp.run_shell_cmd import run_shell_cmd
from icpp.run_dfx_cmd import run_dfx_cmd

from icpp.decorators import requires_native_compiler, requires_pro
from icpp.options_build import (
    to_compile_callback,
    option_to_compile_values_string,
    generate_bindings_callback,
    option_generate_bindings_values_string,
)

# options are: "none", "multi-threading"
CONCURRENCY = "multi-threading"


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
    generate_bindings: Annotated[
        str,
        typer.Option(
            help=(
                f"Generate Javascript bindings "
                f"{option_generate_bindings_values_string}."
            ),
            callback=generate_bindings_callback,
        ),
    ] = "yes",
) -> None:
    """Builds a native debug executable with your systems' Clang compiler.

    Reads 'icpp.toml' file in the current folder, and uses: \n
    (-) C++ & C files from [build-native] AND [build-wasm]\n
    (-) Only the compile flags of [build-native] \n
    (-) Only the link flags of [build-native]
    """
    from icpp import icpp_toml  # pylint: disable = import-outside-toplevel

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
        cmd = f"{cpp_compile_cmd_mine()} -c {file}"
        typer.echo(cmd)
        run_shell_cmd(cmd, cwd=build_path)

    def c_compile_file_mine(file: str) -> None:
        cmd = f"{c_compile_cmd_mine()} -c {file}"
        typer.echo(cmd)
        run_shell_cmd(cmd, cwd=build_path)

    def cpp_compile_file_icpp(file: str) -> None:
        cmd = f"{cpp_compile_cmd_default()} -c {file}"
        typer.echo(cmd)
        run_shell_cmd(cmd, cwd=build_path)

    def c_compile_file_icpp(file: str) -> None:
        cmd = f"{c_compile_cmd_default()} -c {file}"
        typer.echo(cmd)
        run_shell_cmd(cmd, cwd=build_path)

    try:
        # ----------------------------------------------------------------------
        # compile 'mine' C++ files, if we have any
        if len(cpp_files.strip()) > 0:
            if CONCURRENCY == "multi-threading":
                typer.echo("--")
                typer.echo("Compiling your C++ files using multi-threading:")
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
                typer.echo("Compiling your C files using multi-threading:")
                with concurrent.futures.ThreadPoolExecutor() as executor:
                    executor.map(c_compile_file_mine, c_files_list)
            else:
                cmd = f"{c_compile_cmd_mine()} -c {c_files}"

                typer.echo("--")
                typer.echo("Compiling your C files with command:")
                typer.echo(cmd)
                run_shell_cmd(cmd, cwd=build_path)

        if to_compile == "all":
            # ----------------------------------------------------------------------
            # compile the C++ files of the Mock IC, if we have any
            if len(config_default.MOCKIC_CPP_FILES.strip()) > 0:
                if CONCURRENCY == "multi-threading":
                    typer.echo("--")
                    typer.echo(
                        "Compiling C++ files of the Mock IC using multi-threading:"
                    )
                    with concurrent.futures.ThreadPoolExecutor() as executor:
                        executor.map(
                            cpp_compile_file_icpp,
                            config_default.MOCKIC_CPP_FILES_LIST,
                        )
                else:
                    cmd = (
                        f"{cpp_compile_cmd_default()} "
                        f"-c {config_default.MOCKIC_CPP_FILES}"
                    )

                    typer.echo("--")
                    typer.echo("Compiling C++ files of the Mock IC with command:")
                    typer.echo(cmd)
                    run_shell_cmd(cmd, cwd=build_path)

            # ----------------------------------------------------------------------
            # compile the C files of the Mock IC, if we have any
            if len(config_default.MOCKIC_C_FILES.strip()) > 0:
                if CONCURRENCY == "multi-threading":
                    typer.echo("--")
                    typer.echo(
                        "Compiling C++ files of the Mock IC using multi-threading:"
                    )
                    with concurrent.futures.ThreadPoolExecutor() as executor:
                        executor.map(
                            c_compile_file_icpp,
                            config_default.MOCKIC_C_FILES_LIST,
                        )
                else:
                    cmd = (
                        f"{c_compile_cmd_default()} -c {config_default.MOCKIC_C_FILES}"
                    )

                    typer.echo("--")
                    typer.echo("Compiling C files of the Mock IC with command:")
                    typer.echo(cmd)
                    run_shell_cmd(cmd, cwd=build_path)

        # ----------------------------------------------------------------------
        # link it into a native executable
        executable = "mockic.exe"
        cmd = (
            f"{config_default.NATIVE_CPP} "
            f"{config_default.NATIVE_CPP_REQUIRED_FLAGS} "
            f"{cpp_link_flags_defaults_s} "
            f"*.o -o {executable}"
        )

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
    typer.echo("--")
    if generate_bindings.lower() == "no":
        typer.echo("Skipping generation of Javascript bindings from your .did file.")
    else:
        typer.echo("Generating Javascript bindings from your .did file:")
        declarations_path = icpp_toml.icpp_toml_path.parent / "src/declarations"
        typer.echo(f"{declarations_path.resolve()}/{icpp_toml.build_wasm['canister']}")
        run_dfx_cmd("generate")
        try:
            typer.echo("✔️")
        except UnicodeEncodeError:
            typer.echo(" ")

    # ----------------------------------------------------------------------
    typer.echo("--")
