"""Runs the native tests for all canisters"""

import argparse
import sys
from pathlib import Path
import subprocess
import typer
from icpp.memory_guard import DEFAULT_MAX_GB
from icpp.run_shell_cmd import run_shell_cmd

ROOT_PATH = Path(__file__).parent.parent


def main() -> int:
    """build-native and run mockic.exe"""
    canister_paths_1 = list((ROOT_PATH / "test/canisters").glob("canister_*"))
    canister_paths_2 = list((ROOT_PATH / "src/icpp/canisters").glob("*"))
    canister_paths = canister_paths_2 + canister_paths_1

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--max-gb",
        type=float,
        default=DEFAULT_MAX_GB,
        help="memory limit of each native run, in GiB",
    )
    parser.add_argument(
        "--canister",
        action="append",
        help="test only this canister (repeatable); default: all canisters",
    )
    args = parser.parse_args()
    if args.canister:
        unknown = set(args.canister) - {p.name for p in canister_paths}
        if unknown:
            typer.echo(f"ERROR: no canister named {', '.join(sorted(unknown))}")
            return 1
        canister_paths = [p for p in canister_paths if p.name in args.canister]
    for canister_path in canister_paths:
        typer.echo(f"====\nTesting canister: {canister_path.name}")

        configs = [file.name for file in canister_path.glob("*.toml")]
        for config in configs:
            try:
                typer.echo(f"--\nicpp build-native with config {config}")
                run_shell_cmd("icpp build-native --to-compile all", cwd=canister_path)

                typer.echo("--\nRun mockic.exe, under the memory guard")
                run_shell_cmd(
                    f"icpp run-native --max-gb {args.max_gb}", cwd=canister_path
                )

            except subprocess.CalledProcessError as e:
                typer.echo("--\nSomething did not pass")
                return e.returncode

            # For greet canister, also test build-library-native
            if canister_path.name == "greet":
                try:
                    typer.echo(
                        f"--\nBuild all libraries for the greet canister "
                        f"with config {config}"
                    )
                    run_shell_cmd(
                        f"icpp build-library-native --config {config} ",
                        cwd=canister_path,
                    )

                    typer.echo(
                        f"--\nBuild libhello for the greet canister with config {config}"
                    )
                    run_shell_cmd(
                        f"icpp build-library-native --config {config} libhello",
                        cwd=canister_path,
                    )

                    typer.echo(f"--\nicpp build-native with config {config}")
                    run_shell_cmd(
                        "icpp build-native --to-compile mine-no-lib", cwd=canister_path
                    )

                    typer.echo("--\nRun mockic.exe, under the memory guard")
                    run_shell_cmd(
                        f"icpp run-native --max-gb {args.max_gb}", cwd=canister_path
                    )

                except subprocess.CalledProcessError as e:
                    typer.echo("--\nSomething did not pass")
                    return e.returncode

    typer.echo("--\nCongratulations, everything passed!")
    try:
        typer.echo("💯 🎉 🏁")
    except UnicodeEncodeError:
        typer.echo(" ")
    return 0


if __name__ == "__main__":
    sys.exit(main())
