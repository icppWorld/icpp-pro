"""Reads the icpp.toml file"""

import sys
import glob
from pathlib import Path
from typing import Any
import typer

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
icpp_toml_path = Path("icpp.toml")
if not icpp_toml_path.exists():
    typer.echo(
        f"ERROR: There is no `icpp.toml` file in the current working directory: \n"
        f"        {icpp_toml_path.parent.resolve()} \n"
        f"       Maybe you're in the wrong folder, \n"
        f"       or else can you please create one, so I know what to build?"
    )
    sys.exit(1)


def validate(d_in: dict[Any, Any]) -> None:
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


def read_build_wasm_table(d_in: dict[Any, Any]) -> dict[Any, Any]:
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


def read_build_native_table(d_in: dict[Any, Any]) -> dict[Any, Any]:
    """Reads and processes the '[build-native]' table."""
    d: dict[Any, Any] = {}
    icpp_toml_path_dir = icpp_toml_path.parent.resolve()
    d["cpp_include_dirs"] = [icpp_toml_path_dir / "native"]
    d["c_include_dirs"] = [icpp_toml_path_dir, icpp_toml_path_dir / "native"]
    read_build_table_common(d, d_in)
    return d


def read_build_table_common(d: dict[Any, Any], d_in: dict[Any, Any]) -> None:
    """Reads the common fields of '[build-wasm]' & '[build-native]'"""

    # Helper function to expand wildcards
    def expand_paths(patterns: list[str]) -> list[Path]:
        paths: set[Path] = set()  # Using a set to store unique paths
        for pattern in patterns:
            absolute_pattern = (icpp_toml_path.parent / pattern).resolve()
            paths.update(map(Path, glob.glob(str(absolute_pattern))))
        return list(paths)

    d["cpp_paths"] = expand_paths(d_in.get("cpp_paths", []))
    if "cpp_include_dirs" in d.keys():
        d["cpp_include_dirs"].extend(expand_paths(d_in.get("cpp_include_dirs", [])))
    else:
        d["cpp_include_dirs"] = d_in.get("cpp_include_dirs", [])
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
# Read the top level icpp.toml
#

with open(icpp_toml_path, "rb") as f:
    data = tomllib.load(f)

validate(data)
build_wasm: dict[Any, Any] = read_build_wasm_table(data.get("build-wasm", {}))
build_native: dict[Any, Any] = read_build_native_table(data.get("build-native", {}))
