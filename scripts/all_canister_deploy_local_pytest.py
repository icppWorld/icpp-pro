"""Deploys & tests all canisters in a freshly started local network"""

import sys
import shutil
from pathlib import Path
import subprocess
import typer
from icpp.run_shell_cmd import run_shell_cmd
from icpp.run_icp_cmd import run_icp_cmd

SCRIPTS_PATH = Path(__file__).parent
ROOT_PATH = Path(__file__).parent.parent


def network_stop(canister_path: Path) -> None:
    """Stops the canister's project-local network, if it is running.

    `icp network stop` exits non-zero when no network is running, which is not
    an error for us - we only want the network to be down afterwards.
    """
    try:
        run_shell_cmd("icp network stop", capture_output=True, cwd=canister_path)
    except subprocess.CalledProcessError:
        pass


def network_start_clean(canister_path: Path) -> None:
    """Starts a clean, project-local network for the canister.

    icp-cli has no `--clean` flag. A managed network keeps both its replica
    state and its canister id mappings under `.icp/cache`, so removing that
    directory is the equivalent of `dfx start --clean`.
    """
    network_stop(canister_path)
    shutil.rmtree(canister_path / ".icp" / "cache", ignore_errors=True)
    run_icp_cmd("network start --background", cwd=canister_path)


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
                # On Mac & Ubuntu, it is much more flexible, and we test more variations
                typer.echo("--\nStart a clean local network")
                network_start_clean(canister_path)

                typer.echo(f"--\nBuild the wasm with config {config}")
                run_shell_cmd(f"icpp build-wasm --config {config} --to-compile all", cwd=canister_path)

                typer.echo(f"--\nDeploy {canister_path.name}")
                run_icp_cmd("deploy --environment local --yes", cwd=canister_path)

                typer.echo(f"--\nRun pytest on {test_api_path}")
                # Run from the canister directory: that is the icp project root,
                # which is how icp-cli finds icp.yaml and the local network.
                run_shell_cmd(f"pytest -vv --network=local {test_api_path}", cwd=canister_path)

                typer.echo("--\nStop the local network")
                network_stop(canister_path)

                # For greet canister, also test build-library & --config flags
                if canister_path.name == "greet":
                    typer.echo("--\nStart a clean local network")
                    network_start_clean(canister_path)

                    typer.echo(f"--\nBuild all libraries for the greet canister with config {config}")
                    run_shell_cmd(f"icpp build-library --config {config} ", cwd=canister_path)

                    typer.echo(f"--\nBuild libhello for the greet canister with config {config}")
                    run_shell_cmd(f"icpp build-library --config {config} libhello", cwd=canister_path)

                    typer.echo(f"--\nBuild the wasm with config {config}")
                    run_shell_cmd(f"icpp build-wasm --config {config} --to-compile mine-no-lib", cwd=canister_path)

                    typer.echo(f"--\nDeploy {canister_path.name}")
                    run_icp_cmd("deploy --environment local --yes", cwd=canister_path)

                    typer.echo(f"--\nRun pytest on {test_api_path}")
                    run_shell_cmd(f"pytest -vv --network=local {test_api_path}", cwd=canister_path)

                    typer.echo("--\nStop the local network")
                    network_stop(canister_path)

            except subprocess.CalledProcessError as e:
                typer.echo("--\nSomething did not pass")
                network_stop(canister_path)
                return e.returncode

    typer.echo("--\nCongratulations, everything passed!")
    try:
        typer.echo("💯 🎉 🏁")
    except UnicodeEncodeError:
        typer.echo(" ")
    return 0


if __name__ == "__main__":
    sys.exit(main())
