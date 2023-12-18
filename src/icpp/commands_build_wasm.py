"""Handles 'icpp build-wasm' """
# pylint: disable = too-many-statements
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

from icpp.decorators import requires_wasi_sdk
from icpp.options_build import (
    to_compile_callback,
    option_to_compile_values_string,
    generate_bindings_callback,
    option_generate_bindings_values_string,
)

# options are: "none", "multi-threading"
CONCURRENCY = "multi-threading"


@app.command()
@requires_wasi_sdk()
def build_wasm(
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
    """Builds the wasm for a canister, using the wasi-sdk compiler.

    Reads icpp.toml in the current folder; Compiles & builds a wasm file.
    """
    from icpp import icpp_toml  # pylint: disable = import-outside-toplevel

    build_path = icpp_toml.icpp_toml_path.parent / "build"
    typer.echo(f"Build folder: {build_path.resolve()}")

    if to_compile == "all":
        if build_path.exists():
            typer.echo("Deleting the build folder.")
            try:
                shutil.rmtree(build_path)
            except OSError as e:
                typer.echo(f"ERROR: During delete of {build_path}:\n {e.strerror}")

    if not build_path.exists():
        build_path.mkdir()

    cpp_files = icpp_toml.build_wasm["cpp_files"]
    cpp_files_list = icpp_toml.build_wasm["cpp_files_list"]
    cpp_include_flags = icpp_toml.build_wasm["cpp_include_flags"]
    cpp_compile_flags_s = icpp_toml.build_wasm["cpp_compile_flags_s"]

    c_files = icpp_toml.build_wasm["c_files"]
    c_files_list = icpp_toml.build_wasm["c_files_list"]
    c_include_flags = icpp_toml.build_wasm["c_include_flags"]
    c_compile_flags_s = icpp_toml.build_wasm["c_compile_flags_s"]

    cpp_compile_flags_defaults_s = config_default.WASM_CPPFLAGS
    cpp_link_flags_defaults_s = config_default.WASM_LDFLAGS
    c_compile_flags_defaults_s = config_default.WASM_CFLAGS

    if icpp_toml.build_wasm["overwrite_default_CPPFLAGS"]:
        cpp_compile_flags_defaults_s = icpp_toml.build_wasm[
            "cpp_compile_flags_defaults_s"
        ]
    if icpp_toml.build_wasm["overwrite_default_LDFLAGS"]:
        cpp_link_flags_defaults_s = icpp_toml.build_wasm["cpp_link_flags_defaults_s"]
    if icpp_toml.build_wasm["overwrite_default_CFLAGS"]:
        c_compile_flags_defaults_s = icpp_toml.build_wasm["c_compile_flags_defaults_s"]

    def cpp_compile_cmd_default() -> str:
        cmd = (
            f"{config_default.WASM_CPP} {config_default.WASM_CPP_REQUIRED_FLAGS} "
            f"{cpp_compile_flags_defaults_s} "
        )
        return cmd

    def c_compile_cmd_default() -> str:
        cmd = (
            f"{config_default.WASM_C} {config_default.WASM_C_REQUIRED_FLAGS} "
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
            # compile the C++ files of the IC API, if we have any
            if len(config_default.IC_CPP_FILES.strip()) > 0:
                if CONCURRENCY == "multi-threading":
                    typer.echo("--")
                    typer.echo(
                        "Compiling C++ files of the IC_API using multi-threading:"
                    )
                    with concurrent.futures.ThreadPoolExecutor() as executor:
                        executor.map(
                            cpp_compile_file_icpp,
                            config_default.IC_CPP_FILES_LIST,
                        )
                else:
                    cmd = (
                        f"{cpp_compile_cmd_default()} "
                        f"-c {config_default.IC_CPP_FILES}"
                    )

                    typer.echo("--")
                    typer.echo("Compiling C++ files of the IC_API with command:")
                    typer.echo(cmd)
                    run_shell_cmd(cmd, cwd=build_path)

            # ----------------------------------------------------------------------
            # compile the C files of the IC API, if we have any
            if len(config_default.IC_C_FILES.strip()) > 0:
                if CONCURRENCY == "multi-threading":
                    typer.echo("--")
                    typer.echo(
                        "Compiling C++ files of the IC_API using multi-threading:"
                    )
                    with concurrent.futures.ThreadPoolExecutor() as executor:
                        executor.map(
                            c_compile_file_icpp,
                            config_default.IC_C_FILES_LIST,
                        )
                else:
                    cmd = f"{c_compile_cmd_default()} -c {config_default.IC_C_FILES}"

                    typer.echo("--")
                    typer.echo("Compiling C files of the IC_API with command:")
                    typer.echo(cmd)
                    run_shell_cmd(cmd, cwd=build_path)

        # ----------------------------------------------------------------------
        # link it into a wasm
        cmd = (
            f"{config_default.WASM_CPP} "
            f"{config_default.WASM_CPP_REQUIRED_FLAGS} "
            f"{cpp_link_flags_defaults_s} "
            f"*.o -o {icpp_toml.build_wasm['canister']}.wasm"
        )

        typer.echo("--")
        typer.echo("Linking it into a wasm with command:")
        typer.echo(cmd)
        run_shell_cmd(cmd, cwd=build_path)

    except subprocess.CalledProcessError as e:
        sys.exit(e.returncode)
    except Exception as e:  # pylint: disable=broad-except
        typer.echo(
            f"ERROR: something went wrong in module 'commands_build_wasm.py' \n "
            f"Exception is: \n{e}"
        )
        sys.exit(1)

    # ----------------------------------------------------------------------
    # copy did into build folder
    did_path = icpp_toml.build_wasm["did_path"]
    typer.echo("--")
    if not did_path.exists():
        typer.echo(
            f"ERROR: Cannot find the `did_path` '{did_path}' \n"
            f"       Please update your icpp.toml"
        )
        sys.exit(1)

    typer.echo("Copying your Candid file to the build folder...")
    shutil.copy(did_path, build_path)

    # ----------------------------------------------------------------------
    typer.echo("--")
    typer.echo("All done building the canister Wasm module:")
    typer.echo(f"{build_path.resolve()}/{icpp_toml.build_wasm['canister']}.wasm")
    try:
        typer.echo("✔️")
    except UnicodeEncodeError:
        typer.echo(" ")

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
