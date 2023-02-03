"""Handles 'icpp build-wasm' """
import sys
import subprocess
import shutil
import typer

from icpp.__main__ import app
from icpp import config_default
from icpp.run_shell_cmd import run_shell_cmd

from icpp.decorators import requires_wasi_sdk


@app.command()
@requires_wasi_sdk()
def build_wasm() -> None:
    """Builds the wasm for a canister, using the wasi-sdk compiler.

    Reads icpp.toml in the current folder; Compiles & builds a wasm file.
    """
    from icpp import icpp_toml  # pylint: disable = import-outside-toplevel

    build_path = icpp_toml.icpp_toml_path.parent / "build"
    typer.echo(f"Build folder: {build_path.resolve()}")

    if build_path.exists():
        typer.echo("Deleting the build folder.")
        try:
            shutil.rmtree(build_path)
        except OSError as e:
            typer.echo(f"ERROR: During delete of {build_path}:\n {e.strerror}")

    if not build_path.exists():
        build_path.mkdir()

    try:
        # ----------------------------------------------------------------------
        # compile all C++ files, if we have any
        cpp_files = icpp_toml.build_wasm["cpp_files"]
        cpp_compile_flags_s = icpp_toml.build_wasm["cpp_compile_flags_s"]
        if len(cpp_files.strip()) > 0:
            cmd = (
                f"{config_default.WASM_CPP} "
                f"{config_default.WASM_CPPFLAGS} {cpp_compile_flags_s} "
                f"-c {cpp_files}"
            )

            typer.echo("--")
            typer.echo("Compiling your C++ files with command:")
            typer.echo(cmd)
            run_shell_cmd(cmd, cwd=build_path)

        # ----------------------------------------------------------------------
        # compile all C files, if we have any
        c_files = icpp_toml.build_wasm["c_files"]
        c_compile_flags_s = icpp_toml.build_wasm["c_compile_flags_s"]
        if len(c_files.strip()) > 0:
            cmd = (
                f"{config_default.WASM_C} "
                f"{config_default.WASM_CFLAGS} {c_compile_flags_s} "
                f"-c {c_files}"
            )

            typer.echo("--")
            typer.echo("Compiling your C files with command:")
            typer.echo(cmd)
            run_shell_cmd(cmd, cwd=build_path)

        # ----------------------------------------------------------------------
        # compile the C++ files of the IC API, if we have any
        if len(config_default.IC_CPP_FILES.strip()) > 0:
            cmd = (
                f"{config_default.WASM_CPP} "
                f"{config_default.WASM_CPPFLAGS} "
                f"-c {config_default.IC_CPP_FILES}"
            )

            typer.echo("--")
            typer.echo("Compiling C++ files of the IC_API with command:")
            typer.echo(cmd)
            run_shell_cmd(cmd, cwd=build_path)

        # ----------------------------------------------------------------------
        # compile the C files of the IC API, if we have any
        if len(config_default.IC_C_FILES.strip()) > 0:
            cmd = (
                f"{config_default.WASM_C} "
                f"{config_default.WASM_CFLAGS} "
                f"-c {config_default.IC_C_FILES}"
            )

            typer.echo("--")
            typer.echo("Compiling C files of the IC_API with command:")
            typer.echo(cmd)
            run_shell_cmd(cmd, cwd=build_path)

        # ----------------------------------------------------------------------
        # link it into a wasm
        cmd = (
            f"{config_default.WASM_CPP} "
            f"{config_default.WASM_CPPFLAGS} {config_default.WASM_LDFLAGS} "
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
    # All done
    typer.echo("--")
    typer.echo("All done building the canister Wasm module:")
    typer.echo(f"{build_path.resolve()}/{icpp_toml.build_wasm['canister']}.wasm")
    typer.echo("✔️")
    typer.echo("--")
