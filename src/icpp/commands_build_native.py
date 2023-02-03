"""Handles 'icpp build-wasm' """
import sys
import subprocess
import shutil
import typer

from icpp.__main__ import app
from icpp import config_default
from icpp.run_shell_cmd import run_shell_cmd

from icpp.decorators import requires_native_compiler


@app.command()
@requires_native_compiler()
def build_native() -> None:
    """Builds a native debug executable with your systems' Clang compiler.

    Reads 'icpp.toml' file in the current folder, and uses: \n
    (-) C++ & C files from [build-native] AND [build-wasm]\n
    (-) Only the compile flags of [build-native] \n
    (-) Only the link flags of [build-native]
    """
    from icpp import icpp_toml  # pylint: disable = import-outside-toplevel

    build_path = icpp_toml.icpp_toml_path.parent / "build-native"
    typer.echo(f"Build folder: {build_path.resolve()}")

    if build_path.exists():
        typer.echo("Deleting the build-native folder.")
        try:
            shutil.rmtree(build_path)
        except OSError as e:
            typer.echo(f"ERROR: During delete of {build_path}:\n {e.strerror}")

    if not build_path.exists():
        build_path.mkdir()

    try:
        # ----------------------------------------------------------------------
        # compile all C++ files, if we have any
        cpp_files = (
            icpp_toml.build_native["cpp_files"] + icpp_toml.build_wasm["cpp_files"]
        )
        cpp_compile_flags_s = icpp_toml.build_native["cpp_compile_flags_s"]
        if len(cpp_files.strip()) > 0:
            cmd = (
                f"{config_default.NATIVE_CPP} "
                f"{config_default.NATIVE_CPPFLAGS} {cpp_compile_flags_s} "
                f"-c {cpp_files}"
            )

            typer.echo("--")
            typer.echo("Compiling your C++ files with command:")
            typer.echo(cmd)
            run_shell_cmd(cmd, cwd=build_path)

        # ----------------------------------------------------------------------
        # compile all C files, if we have any
        c_files = icpp_toml.build_native["c_files"] + icpp_toml.build_wasm["c_files"]
        c_compile_flags_s = icpp_toml.build_native["c_compile_flags_s"]
        if len(c_files.strip()) > 0:
            cmd = (
                f"{config_default.NATIVE_C} "
                f"{config_default.NATIVE_CFLAGS} {c_compile_flags_s} "
                f"-c {c_files}"
            )

            typer.echo("--")
            typer.echo("Compiling your C files with command:")
            typer.echo(cmd)
            run_shell_cmd(cmd, cwd=build_path)

        # ----------------------------------------------------------------------
        # compile the C++ files of the Mock IC, if we have any
        if len(config_default.MOCKIC_CPP_FILES.strip()) > 0:
            cmd = (
                f"{config_default.NATIVE_CPP} "
                f"{config_default.NATIVE_CPPFLAGS} "
                f"-c {config_default.MOCKIC_CPP_FILES}"
            )

            typer.echo("--")
            typer.echo("Compiling C++ files of the Mock IC with command:")
            typer.echo(cmd)
            run_shell_cmd(cmd, cwd=build_path)

        # ----------------------------------------------------------------------
        # compile the C files of the Mock IC, if we have any
        if len(config_default.MOCKIC_C_FILES.strip()) > 0:
            cmd = (
                f"{config_default.NATIVE_C} "
                f"{config_default.NATIVE_CFLAGS} "
                f"-c {config_default.MOCKIC_C_FILES}"
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
            f"{config_default.NATIVE_CPPFLAGS} {config_default.NATIVE_LDFLAGS} "
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
    typer.echo("(✔)You can run it from the command line")
    typer.echo("(✔)You can debug it with VS Code + CodeLLDB")
    typer.echo("--")
