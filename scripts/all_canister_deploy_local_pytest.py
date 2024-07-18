"""Deploys & tests all canisters in a freshly started local network"""

import sys
import platform
from pathlib import Path
import subprocess
import typer
from icpp.run_shell_cmd import run_shell_cmd
from icpp.run_dfx_cmd import run_dfx_cmd

SCRIPTS_PATH = Path(__file__).parent
ROOT_PATH = Path(__file__).parent.parent

def main() -> int:
    """Start local network; Deploy canister; Pytest"""
    canister_paths_1 = list((ROOT_PATH / "test/canisters").glob("canister_*"))
    canister_paths_2 = list((ROOT_PATH / "src/icpp/canisters").glob("*"))
    canister_paths = canister_paths_2 + canister_paths_1
    for canister_path in canister_paths:
        typer.echo(f"====\nTesting canister: {canister_path.name}")

        test_api_path = canister_path / "test/test_apis.py"
        configs = [file.name for file in canister_path.glob("*.toml")]
        for config in configs:
            try:
                if platform.system() == "Windows":
                    # On Windows, we run it all within a single Powershell script
                    if config == "icpp.toml":
                        run_shell_cmd(SCRIPTS_PATH / "smoketest.ps1", run_in_powershell=True, cwd=canister_path)
                else:
                    # On Mac & Ubuntu, it is much more flexible, and we test more variations
                    typer.echo("--\nStop the local network")
                    run_dfx_cmd("stop")

                    typer.echo("--\nStart a clean local network")
                    run_dfx_cmd("start --clean --background")

                    typer.echo(f"--\nBuild the wasm with config {config}")
                    run_shell_cmd(f"icpp build-wasm --config {config} --to-compile all", cwd=canister_path)

                    typer.echo(f"--\nDeploy {canister_path.name}")
                    run_dfx_cmd("deploy", cwd=canister_path)

                    typer.echo(f"--\nRun pytest on {test_api_path}")
                    run_shell_cmd(f"pytest -vv --network=local {test_api_path}", cwd=ROOT_PATH)

                    typer.echo("--\nStop the local network")
                    run_dfx_cmd("stop")

            
                # For greet canister, also test build-library & --config flags
                if canister_path.name == "greet":
                    if platform.system() == "Windows":
                        if config == "icpp-c++17.toml":
                            run_shell_cmd(SCRIPTS_PATH / "smoketest-greet-c++17.ps1", run_in_powershell=True, cwd=canister_path)
                    else:
                        typer.echo("--\nStop the local network")
                        run_dfx_cmd("stop")

                        typer.echo("--\nStart a clean local network")
                        run_dfx_cmd("start --clean --background")

                        typer.echo(f"--\nBuild all libraries for the greet canister with config {config}")
                        run_shell_cmd(f"icpp build-library --config {config} ", cwd=canister_path)

                        typer.echo(f"--\nBuild libhello for the greet canister with config {config}")
                        run_shell_cmd(f"icpp build-library --config {config} libhello", cwd=canister_path)

                        typer.echo(f"--\nBuild the wasm with config {config}")
                        run_shell_cmd(f"icpp build-wasm --config {config} --to-compile mine-no-lib", cwd=canister_path)

                        typer.echo(f"--\nDeploy {canister_path.name}")
                        run_dfx_cmd("deploy", cwd=canister_path)

                        typer.echo(f"--\nRun pytest on {test_api_path}")
                        run_shell_cmd(f"pytest -vv --network=local {test_api_path}", cwd=ROOT_PATH)

                        typer.echo("--\nStop the local network")
                        run_dfx_cmd("stop")

            except subprocess.CalledProcessError as e:
                typer.echo("--\nSomething did not pass")
                if platform.system() != "Windows":
                    run_dfx_cmd("stop")
                return e.returncode

    typer.echo("--\nCongratulations, everything passed!")
    try:
        typer.echo("💯 🎉 🏁")
    except UnicodeEncodeError:
        typer.echo(" ")
    return 0


if __name__ == "__main__":
    sys.exit(main())
