"""Handles 'icpp build-wasm' """

# pylint: disable = too-many-statements
import sys
import os
import json
from pathlib import Path
import subprocess
import shutil
import concurrent.futures
import typer
from typing_extensions import Annotated

from icpp import __version__
from icpp.__main__ import app

from icpp import config_default
from icpp.run_shell_cmd import run_shell_cmd
from icpp.run_dfx_cmd import run_dfx_cmd

from icpp.decorators import requires_wasi_sdk, requires_rust, requires_native_compiler
from icpp.options_build import (
    config_callback,
    to_compile_callback,
    option_to_compile_values_string,
    generate_bindings_callback,
    option_generate_bindings_values_string,
)
from icpp.commands_build_library import build_library
from icpp.call_function_from_name import call_function

# options are: "none", "multi-threading"
CONCURRENCY = "multi-threading"


@app.command()
@requires_native_compiler()
@requires_rust()
@requires_wasi_sdk()
def build_wasm(
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
    """Builds the wasm."""

    config_default.ICPP_TOML_PATH = Path(config)
    from icpp import icpp_toml  # pylint: disable = import-outside-toplevel

    # ----------------------------------------------------------------------
    # First build the libraries
    if to_compile != "mine-no-lib":
        build_library(config)

    # ----------------------------------------------------------------------
    typer.echo("----------------------------")
    typer.echo("Building the wasm...")

    # ----------------------------------------------------------------------
    build_path = icpp_toml.icpp_toml_path.parent / "build"
    typer.echo(f"Build folder: {build_path.resolve()}")

    if to_compile == "all":
        if build_path.exists():
            typer.echo(f"Deleting the folder: {build_path}")
            try:
                shutil.rmtree(build_path)
            except OSError as e:
                typer.echo(f"ERROR: During delete of {build_path}:\n {e.strerror}")

    if not build_path.exists():
        build_path.mkdir()

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

    cpp_files = icpp_toml.build_wasm["cpp_files"]
    cpp_files_list = icpp_toml.build_wasm["cpp_files_list"]
    cpp_include_flags = icpp_toml.build_wasm["cpp_include_flags"]
    cpp_compile_flags_s = icpp_toml.build_wasm["cpp_compile_flags_s"]
    cpp_link_flags_s = icpp_toml.build_wasm["cpp_link_flags_s"]

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
        # ensure unique *.o names, even if file has same name in a subdirectory
        # pylint: disable=no-member
        output_file = (
            f"{file}".replace(f"{os.getcwd()}", "").replace("/", "_").replace("\\", "_")
        )
        cmd = f"{cpp_compile_cmd_mine()} -c {file} -o {output_file}.o"
        typer.echo(file)
        run_shell_cmd(cmd, cwd=build_path)

    def c_compile_file_mine(file: str) -> None:
        # ensure unique *.o names, even if file has same name in a
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
        # link it into a wasi compatible wasm

        cmd = (
            f"{config_default.WASM_CPP} "
            f"{config_default.WASM_CPP_REQUIRED_FLAGS} "
            f"{cpp_link_flags_defaults_s} "
            f"{cpp_link_flags_s} "
            f"*.o "
        )

        # The rust system libraries
        cmd += f"{config_default.IC_WASI_POLYFILL} "

        # The project libraries
        for library in icpp_toml.libraries:
            full_lib_name = f"{library['lib_name']}{config_default.WASM_AR_EXT}"
            full_lib_path = (
                icpp_toml.icpp_toml_path.parent
                / "build-library"
                / library["lib_name"]
                / full_lib_name
            ).resolve()
            cmd += f"{full_lib_path} "

        cmd += f"-o {icpp_toml.build_wasm['canister']}_wasi.wasm "

        typer.echo("--")
        typer.echo("Building a wasi compatible wasm with command:")
        typer.echo(cmd)
        run_shell_cmd(cmd, cwd=build_path)

        # ----------------------------------------------------------------------
        # converting it into an IC compatible wasm using wasi2ic

        cmd = (
            f"{config_default.WASI2IC} "
            f"{icpp_toml.build_wasm['canister']}_wasi.wasm "
            f"{icpp_toml.build_wasm['canister']}.wasm "
        )
        typer.echo("--")
        typer.echo("Converting into an IC compatible wasm with command:")
        typer.echo(cmd)
        run_shell_cmd(cmd, cwd=build_path)

        # ----------------------------------------------------------------------
        # Call optional user provided function

        post_wasm_function = icpp_toml.build_wasm["post_wasm_function"]
        if post_wasm_function is not None and post_wasm_function.strip() != "":
            typer.echo("--")
            typer.echo(f"Calling post_wasm_function: {post_wasm_function}")

            call_function(post_wasm_function)

        # ----------------------------------------------------------------------
        # add custom sections:
        # https://forum.dfinity.org/t/rfc-canister-metadata-standard/16280/25?u=icpp

        # icp:public candid:service ...the candid file...
        # icp:private icpp:compiler ...version number...

        typer.echo("--")
        typer.echo("Adding custom sections to the wasm file...")

        # ---
        cmd = (
            f"{config_default.LLVM_OBJCOPY} "
            f'--add-section="icp:public candid:service"="{str(did_path)}" '
            f"{icpp_toml.build_wasm['canister']}.wasm "
            f"{icpp_toml.build_wasm['canister']}.wasm"
        )
        typer.echo(cmd)
        run_shell_cmd(cmd, cwd=build_path)

        # ---
        file_path = build_path / "custom_section_cdk_name.txt"
        with open(file_path, "w", encoding="utf-8") as file:
            file.write("icpp-pro")
        cmd = (
            f"{config_default.LLVM_OBJCOPY} "
            f'--add-section="icp:public cdk:name"="{str(file_path.resolve())}" '
            f"{icpp_toml.build_wasm['canister']}.wasm "
            f"{icpp_toml.build_wasm['canister']}.wasm"
        )
        typer.echo(cmd)
        run_shell_cmd(cmd, cwd=build_path)

        # ---
        file_path = build_path / "custom_section_cdk_languages.txt"
        with open(file_path, "w", encoding="utf-8") as file:
            file.write("c++")
        cmd = (
            f"{config_default.LLVM_OBJCOPY} "
            f'--add-section="icp:public cdk:languages"="{str(file_path.resolve())}" '
            f"{icpp_toml.build_wasm['canister']}.wasm "
            f"{icpp_toml.build_wasm['canister']}.wasm"
        )
        typer.echo(cmd)
        run_shell_cmd(cmd, cwd=build_path)

        # ---
        file_path = build_path / "custom_section_version.txt"
        with open(file_path, "w", encoding="utf-8") as file:
            file.write(__version__)
        cmd = (
            f"{config_default.LLVM_OBJCOPY} "
            f'--add-section="icp:private icpp:compiler"="{str(file_path.resolve())}" '
            f"{icpp_toml.build_wasm['canister']}.wasm "
            f"{icpp_toml.build_wasm['canister']}.wasm"
        )
        typer.echo(cmd)
        run_shell_cmd(cmd, cwd=build_path)

        # ---
        # dfx.tech_stack
        file_path = build_path / "custom_section_dfx.txt"
        with open(file_path, "w", encoding="utf-8") as file:
            file.write(
                '{ "tech_stack": { "language": {"c++": {} }, "cdk": {"icpp-pro": {} } } }'  # pylint: disable=line-too-long
            )
        cmd = (
            f"{config_default.LLVM_OBJCOPY} "
            f'--add-section="icp:public dfx"="{str(file_path.resolve())}" '
            f"{icpp_toml.build_wasm['canister']}.wasm "
            f"{icpp_toml.build_wasm['canister']}.wasm"
        )
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
        run_dfx_cmd("generate", capture_output=True)
        try:
            typer.echo("✔️")
        except UnicodeEncodeError:
            typer.echo(" ")

    # ----------------------------------------------------------------------
    typer.echo("-----")

    icpp_pro_messaging_canister_message = (
        config_default.ICPP_PRO_MESSAGING_CANISTER_DEFAULT_MESSAGE
    )
    try:
        cmd = (
            f"dfx canister call {config_default.ICPP_PRO_MESSAGING_CANISTER} "
            f"get_message --ic --output json"
        )
        response_str = run_shell_cmd(cmd, capture_output=True)
        if response_str is not None:
            response = json.loads(response_str)
            if "Ok" in response and "message" in response["Ok"]:
                icpp_pro_messaging_canister_message = response["Ok"]["message"]
    except:  # pylint: disable=bare-except
        pass

    typer.echo(icpp_pro_messaging_canister_message)
    # ----------------------------------------------------------------------
    typer.echo("-----")
