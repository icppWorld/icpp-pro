"""Handles 'icpp build-library-native' """

# pylint: disable = too-many-statements
import sys
import copy
import subprocess
import shutil
import concurrent.futures
from typing import Optional
from pathlib import Path
import typer
from typing_extensions import Annotated

from icpp import __version__
from icpp.__main__ import app

from icpp import config_default
from icpp.run_shell_cmd import run_shell_cmd

from icpp.decorators import requires_native_compiler
from icpp.options_build import (
    config_callback,
)

# options are: "none", "multi-threading"
CONCURRENCY = "multi-threading"


@app.command()
@requires_native_compiler()
def build_library_native(
    config: Annotated[
        str,
        typer.Option(
            help="Name of configuration toml file.",
            callback=config_callback,
        ),
    ] = "icpp.toml",
    lib_name: Annotated[
        Optional[str],
        typer.Argument(
            help="lib_name to build. If not specified, all libraries are built."
        ),
    ] = None,
) -> None:
    """Builds one or more native static libraries, with your systems' Clang compiler.

    Reads 'icpp.toml' file in the current folder, and uses: \n
    (-) C++ & C files from [[build-library-native]] AND [[build-library]]\n
    (-) Only the compile flags of [build-library-native]
    """
    config_default.ICPP_TOML_PATH = Path(config)
    from icpp import icpp_toml  # pylint: disable = import-outside-toplevel

    typer.echo("----------------------------")
    typer.echo("Building libraries...")

    # ----------------------------------------------------------------------
    # build-library-native
    build_root_path = icpp_toml.icpp_toml_path.parent / "build-library-native"
    # typer.echo(f"Build folder: {build_root_path.resolve()}")

    if lib_name is None:
        if build_root_path.exists():
            typer.echo(f"Deleting the folder: {build_root_path} ")
            try:
                shutil.rmtree(build_root_path)
            except OSError as e:
                typer.echo(f"ERROR: During delete of {build_root_path}:\n {e.strerror}")

    if not build_root_path.exists():
        build_root_path.mkdir()

    # ----------------------------------------------------------------------
    # build-library/lib_name

    cpp_compile_flags_defaults_s = config_default.NATIVE_CPPFLAGS
    # cpp_link_flags_defaults_s = config_default.NATIVE_LDFLAGS
    c_compile_flags_defaults_s = config_default.NATIVE_CFLAGS

    def cpp_compile_file_mine(file: str, cpp_compile_cmd: str, path: Path) -> None:
        cmd = f"{cpp_compile_cmd} -c {file}"
        typer.echo(cmd)
        run_shell_cmd(cmd, cwd=path)

    def c_compile_file_mine(file: str, c_compile_cmd: str, path: Path) -> None:
        cmd = f"{c_compile_cmd} -c {file}"
        typer.echo(cmd)
        run_shell_cmd(cmd, cwd=path)

    # loop over the [[build-library]], then add routines of [[build-library-native]];
    # use flags of [[build-library-native]]
    for lib in icpp_toml.libraries:
        # when we apply the hack below, we do not want to overwrite reference data
        library = copy.deepcopy(lib)

        if (lib_name is None) or (lib_name == library["lib_name"]):
            # Find the library-native with same name, if it was provided
            library_native = None
            for lib_native in icpp_toml.libraries_native:
                if lib_native["lib_name"] == library["lib_name"]:
                    library_native = lib_native
                    break

            # hack for __ic_candid__ library. We use ONLY the native files
            if library["lib_name"] == "__ic_candid__":
                library["cpp_files_list"] = []
                library["c_files_list"] = []
                library["cpp_files"] = ""
                library["cpp_header_files"] = ""
                library["c_files"] = ""

            # create the build folder
            full_lib_name = f"{library['lib_name']}{config_default.NATIVE_AR_EXT}"

            build_path = build_root_path / library["lib_name"]
            typer.echo(f"Native library build folder: {build_path.resolve()}")

            if build_path.exists():
                # typer.echo("Deleting the build folder.")
                try:
                    shutil.rmtree(build_path)
                except OSError as e:
                    typer.echo(f"ERROR: During delete of {build_path}:\n {e.strerror}")

            if not build_path.exists():
                build_path.mkdir()

            # ----------------------------------------------------------------------

            cpp_files = library["cpp_files"]
            if library_native:
                cpp_files += library_native["cpp_files"]

            cpp_files_list = library["cpp_files_list"]
            if library_native:
                cpp_files_list += library_native["cpp_files_list"]

            cpp_include_flags = library["cpp_include_flags"]
            if library_native:
                cpp_include_flags += library_native["cpp_include_flags"]

            cpp_compile_flags_s = ""
            if library_native:
                cpp_compile_flags_s += library_native["cpp_compile_flags_s"]

            c_files = library["c_files"]
            if library_native:
                c_files += library_native["c_files"]

            c_files_list = library["c_files_list"]
            if library_native:
                c_files_list += library_native["c_files_list"]

            c_include_flags = library["c_include_flags"]
            if library_native:
                c_include_flags += library_native["c_include_flags"]

            c_compile_flags_s = ""
            if library_native:
                c_compile_flags_s += library_native["c_compile_flags_s"]

            if library_native:
                if library_native["overwrite_default_CPPFLAGS"]:
                    cpp_compile_flags_defaults_s = library_native[
                        "cpp_compile_flags_defaults_s"
                    ]
                if library_native["overwrite_default_CFLAGS"]:
                    c_compile_flags_defaults_s = library_native[
                        "c_compile_flags_defaults_s"
                    ]

            cpp_compile_cmd_mine = (
                f"{config_default.NATIVE_CPP} "
                f"{config_default.NATIVE_CPP_REQUIRED_FLAGS} "
                f"{cpp_compile_flags_defaults_s} "
                f"{cpp_include_flags} "
                f"{cpp_compile_flags_s} "
            )

            c_compile_cmd_mine = (
                f"{config_default.NATIVE_C} "
                f"{config_default.NATIVE_C_REQUIRED_FLAGS} "
                f"{c_compile_flags_defaults_s} "
                f"{c_include_flags} {c_compile_flags_s} "
            )

            try:
                # ----------------------------------------------------------------------
                # compile C++ files, if we have any
                if len(cpp_files.strip()) > 0:
                    if CONCURRENCY == "multi-threading":
                        typer.echo("--")
                        typer.echo(
                            f"Compiling C++ files for library: {library['lib_name']}"
                        )
                        with concurrent.futures.ThreadPoolExecutor() as executor:
                            executor.map(
                                lambda file, cpp_compile_cmd=cpp_compile_cmd_mine, build_path=build_path: cpp_compile_file_mine(  # pylint: disable=line-too-long
                                    file, cpp_compile_cmd, build_path
                                ),
                                cpp_files_list,
                            )
                    else:
                        cmd = f"{cpp_compile_cmd_mine} -c {cpp_files}"

                        typer.echo("--")
                        typer.echo(
                            f"Compiling C++ files for library: {library['lib_name']}"
                        )
                        typer.echo(cmd)
                        run_shell_cmd(cmd, cwd=build_path)

                # ----------------------------------------------------------------------
                # compile C files, if we have any
                if len(c_files.strip()) > 0:
                    if CONCURRENCY == "multi-threading":
                        typer.echo("--")
                        typer.echo(
                            f"Compiling C files for library: {library['lib_name']}"
                        )
                        with concurrent.futures.ThreadPoolExecutor() as executor:
                            executor.map(
                                lambda file, c_compile_cmd=c_compile_cmd_mine, build_path=build_path: c_compile_file_mine(  # pylint: disable=line-too-long
                                    file, c_compile_cmd, build_path
                                ),
                                c_files_list,
                            )
                    else:
                        cmd = f"{c_compile_cmd_mine} -c {c_files}"

                        typer.echo("--")
                        typer.echo(
                            f"Compiling C files for library: {library['lib_name']}"
                        )
                        typer.echo(cmd)
                        run_shell_cmd(cmd, cwd=build_path)

                # ----------------------------------------------------------------------
                # create the library
                cmd = (
                    f"{config_default.NATIVE_AR} "
                    f"{config_default.NATIVE_ARFLAGS} "
                    f"{full_lib_name} *.o"
                )

                typer.echo("--")
                typer.echo(f"Creating the static library: {full_lib_name}")
                typer.echo(cmd)
                run_shell_cmd(cmd, cwd=build_path)

            except subprocess.CalledProcessError as e:
                sys.exit(e.returncode)
            except Exception as e:  # pylint: disable=broad-except
                typer.echo(
                    f"ERROR: something went wrong in module "
                    f"'commands_build_library_native.py' \n "
                    f"Exception is: \n{e}"
                )
                sys.exit(1)

            # ----------------------------------------------------------------------
            typer.echo("--")
            typer.echo("All done building the native static library:")
            typer.echo(f"{build_path.resolve()}/{full_lib_name}")
            try:
                typer.echo("✔️")
            except UnicodeEncodeError:
                typer.echo(" ")

            # ----------------------------------------------------------------------
            typer.echo("-----")
