"""Reads the icpp.toml file"""

import sys
import glob
from pathlib import Path
from typing import Any, Dict, List, Set
import typer
from icpp import config_default

# `tomllib` was introduced in python 3.11
# for earlier versions, use `tomli` instead
if sys.version_info >= (3, 11):
    import tomllib
else:
    try:
        import tomli as tomllib
    except ImportError:
        typer.echo("ERROR: cannot find python module tomli")
        sys.exit(1)

# Look for `icpp.toml`in the current folder
icpp_toml_path = config_default.ICPP_TOML_PATH
if not icpp_toml_path.exists():
    typer.echo(
        f"ERROR: There is no {config_default.ICPP_TOML_PATH} file in the "
        f"current working directory: \n"
        f"        {icpp_toml_path.parent.resolve()} \n"
        f"       Maybe you're in the wrong folder, \n"
        f"       or else can you please create one, so I know what to build?"
    )
    sys.exit(1)


def validate(d_in: Dict[Any, Any]) -> None:
    """Validates if required fields are present"""
    if "build-wasm" not in d_in.keys():
        typer.echo(
            f"ERROR: Missing table '[build-wasm]'\n"
            f"       {icpp_toml_path.resolve()}"
        )
        sys.exit(1)

    dwasm = d_in.get("build-wasm", {})
    if "canister" not in dwasm.keys():
        typer.echo(
            f"ERROR: Missing key 'canister' in '[build-wasm]' table.\n"
            f"       {icpp_toml_path.resolve()}"
        )
        sys.exit(1)

    if "did_path" not in dwasm.keys():
        typer.echo(
            f"ERROR: Missing key 'did_path' in '[build-wasm]' table.\n"
            f"       {icpp_toml_path.resolve()}"
        )
        sys.exit(1)


def read_build_wasm_table(d_in: Dict[Any, Any]) -> Dict[Any, Any]:
    """Reads and processes the '[build-wasm]' table."""
    d = {}
    d["canister"] = d_in.get("canister", "default")
    d["did_path"] = icpp_toml_path.parent / Path(d_in.get("did_path", "")).resolve()
    d["did_file"] = str(d["did_path"]) + " "

    icpp_toml_path_dir = icpp_toml_path.parent.resolve()
    d["cpp_include_dirs"] = [icpp_toml_path_dir, icpp_toml_path_dir / "src"]
    d["c_include_dirs"] = [icpp_toml_path_dir, icpp_toml_path_dir / "src"]
    read_build_table_common(d, d_in)

    return d


def read_libraries(
    d_in: List[Dict[Any, Any]],
    build_wasm_: Dict[Any, Any],
    build_native_: Dict[Any, Any],
    native: bool = False,
) -> List[Dict[Any, Any]]:
    """Reads and processes the '[[build-library]]' tables."""
    libs = []
    seen_lib_names = set()

    # First add the libraries from icpp.toml
    for library_in in d_in:
        library = {}

        library["lib_name"] = library_in.get("lib_name", "")
        if library["lib_name"] in seen_lib_names:
            typer.echo(
                f"ERROR: duplicate 'lib_name' found: {library['lib_name']}\n"
                f"       {icpp_toml_path.resolve()}"
            )
            sys.exit(1)
        seen_lib_names.add(library["lib_name"])

        read_build_table_common(library, library_in)
        libs.append(library)

    # If not defined in the icpp.toml, add the library for the IC & CANDID files
    if "__ic_candid__" not in seen_lib_names:
        library = library_ic_candid(build_wasm_, build_native_, native)
        seen_lib_names.add(library["lib_name"])
        libs.append(library)

    return libs


def read_build_native_table(d_in: Dict[Any, Any]) -> Dict[Any, Any]:
    """Reads and processes the '[build-native]' table."""
    d: Dict[Any, Any] = {}
    icpp_toml_path_dir = icpp_toml_path.parent.resolve()
    d["cpp_include_dirs"] = [icpp_toml_path_dir / "native"]
    d["c_include_dirs"] = [icpp_toml_path_dir, icpp_toml_path_dir / "native"]
    read_build_table_common(d, d_in)
    return d


def library_ic_candid(
    build_wasm_: Dict[Any, Any], build_native_: Dict[Any, Any], native: bool = False
) -> Dict[Any, Any]:
    """Creates a dict for a static library contain the IC and CANDID files"""

    d: Dict[Any, Any] = {}

    if native:
        d = build_native_.copy()
        d["cpp_files_list"] = config_default.MOCKIC_CPP_FILES_LIST
        d["c_files_list"] = config_default.MOCKIC_C_FILES_LIST
        d["cpp_files"] = config_default.MOCKIC_CPP_FILES
        d["cpp_header_files"] = config_default.MOCKIC_HEADER_FILES
        d["c_files"] = config_default.MOCKIC_C_FILES
    else:
        d = build_wasm_.copy()
        d["cpp_files_list"] = config_default.IC_CPP_FILES_LIST
        d["c_files_list"] = config_default.IC_C_FILES_LIST
        d["cpp_files"] = config_default.IC_CPP_FILES
        d["cpp_header_files"] = config_default.IC_HEADER_FILES
        d["c_files"] = config_default.IC_C_FILES

    d["lib_name"] = "__ic_candid__"

    # if non-empty, these defaults will overwrite internal settings
    # d["cpp_compile_flags_defaults"] = []
    # d["c_compile_flags_defaults"] = []

    #
    # all in one compiler uses a long string of strings, not a list
    #
    d["cpp_include_flags"] = ""
    # d["cpp_compile_flags_s"] = ""
    d["c_include_flags"] = ""
    # d["c_header_files"] = ""
    # d["c_compile_flags_s"] = ""

    #
    # overwriting defaults also uses strings
    #
    # d["cpp_compile_flags_defaults_s"] = ""
    # d["c_compile_flags_defaults_s"] = ""

    # d["overwrite_default_CFLAGS"] = False
    # d["overwrite_default_CPPFLAGS"] = False
    # d["overwrite_default_LDFLAGS"] = False

    return d


def read_build_table_common(d: Dict[Any, Any], d_in: Dict[Any, Any]) -> None:
    """Reads the common fields of '[build-wasm]' & '[build-native]'"""

    # Helper function to expand wildcards
    def expand_paths(patterns: List[str]) -> List[Path]:
        paths: Set[Path] = set()  # Using a set to store unique paths
        for pattern in patterns:
            absolute_pattern = (icpp_toml_path.parent / pattern).resolve()
            paths.update(map(Path, glob.glob(str(absolute_pattern))))
        return list(paths)

    d["cpp_paths"] = expand_paths(d_in.get("cpp_paths", []))
    if "cpp_include_dirs" in d.keys():
        d["cpp_include_dirs"].extend(expand_paths(d_in.get("cpp_include_dirs", [])))
    else:
        d["cpp_include_dirs"] = expand_paths(d_in.get("cpp_include_dirs", []))
    d["cpp_header_paths"] = expand_paths(d_in.get("cpp_header_paths", []))
    d["cpp_compile_flags"] = d_in.get("cpp_compile_flags", [])
    d["cpp_link_flags"] = d_in.get("cpp_link_flags", [])

    d["c_paths"] = expand_paths(d_in.get("c_paths", []))
    if "c_include_dirs" in d.keys():
        d["c_include_dirs"].extend(expand_paths(d_in.get("c_include_dirs", [])))
    else:
        d["c_include_dirs"] = expand_paths(d_in.get("c_include_dirs", []))

    d["c_header_paths"] = expand_paths(d_in.get("c_header_paths", []))
    d["c_compile_flags"] = d_in.get("c_compile_flags", [])

    # if non-empty, these defaults will overwrite internal settings
    d["cpp_compile_flags_defaults"] = d_in.get("cpp_compile_flags_defaults", [])
    d["cpp_link_flags_defaults"] = d_in.get("cpp_link_flags_defaults", [])
    d["c_compile_flags_defaults"] = d_in.get("c_compile_flags_defaults", [])

    #
    # concurrent compiler uses a list of strings for the files to compile
    #
    d["cpp_files_list"] = [str(x) for x in d["cpp_paths"]]

    d["c_files_list"] = [str(x) for x in d["c_paths"]]

    #
    # all in one compiler uses a long string of strings, not a list
    #
    d["cpp_files"] = " ".join([str(x) for x in d["cpp_paths"]]) + " "
    d["cpp_include_flags"] = (
        " ".join(["-I " + str(path) for path in d["cpp_include_dirs"]]) + " "
    )
    d["cpp_header_files"] = " ".join([str(x) for x in d["cpp_header_paths"]]) + " "
    d["cpp_compile_flags_s"] = " ".join([str(x) for x in d["cpp_compile_flags"]]) + " "

    d["c_files"] = " ".join([str(x) for x in d["c_paths"]]) + " "
    d["c_include_flags"] = (
        " ".join(["-I " + str(path) for path in d["c_include_dirs"]]) + " "
    )
    d["c_header_files"] = " ".join([str(x) for x in d["c_header_paths"]]) + " "
    d["c_compile_flags_s"] = " ".join([str(x) for x in d["c_compile_flags"]]) + " "

    #
    # linker uses a long string of strings, not a list
    #
    d["cpp_link_flags_s"] = " ".join([str(x) for x in d["cpp_link_flags"]]) + " "

    #
    # overwriting defaults also uses strings
    #
    d["cpp_compile_flags_defaults_s"] = (
        " ".join([str(x) for x in d["cpp_compile_flags_defaults"]]) + " "
    )
    d["c_compile_flags_defaults_s"] = (
        " ".join([str(x) for x in d["c_compile_flags_defaults"]]) + " "
    )
    d["cpp_link_flags_defaults_s"] = (
        " ".join([str(x) for x in d["cpp_link_flags_defaults"]]) + " "
    )

    d["overwrite_default_CFLAGS"] = False
    d["overwrite_default_CPPFLAGS"] = False
    d["overwrite_default_LDFLAGS"] = False
    if len(d["c_compile_flags_defaults"]) > 0:
        d["overwrite_default_CFLAGS"] = True
    if len(d["cpp_compile_flags_defaults"]) > 0:
        d["overwrite_default_CPPFLAGS"] = True
    if len(d["cpp_link_flags_defaults"]) > 0:
        d["overwrite_default_LDFLAGS"] = True


#
# Read the top level icpp.toml
#

with open(icpp_toml_path, "rb") as f:
    data = tomllib.load(f)

validate(data)

build_wasm: Dict[Any, Any] = read_build_wasm_table(data.get("build-wasm", {}))
build_native: Dict[Any, Any] = read_build_native_table(data.get("build-native", {}))
libraries: List[Dict[Any, Any]] = read_libraries(
    data.get("build-library", []), build_wasm, build_native, native=False
)
libraries_native: List[Dict[Any, Any]] = read_libraries(
    data.get("build-library-native", []), build_wasm, build_native, native=True
)
