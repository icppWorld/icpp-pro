"""Deploys & tests all canisters in a freshly started local network"""

import sys
from pathlib import Path
import subprocess
import typer
from icpp.run_shell_cmd import run_shell_cmd
from icpp.run_dfx_cmd import run_dfx_cmd


ROOT_PATH = Path(__file__).parent.parent

def main() -> int:
    """Start local network; Deploy canister; Pytest"""
    canister_paths_1 = list((ROOT_PATH / "test/canisters").glob("canister_*"))
    canister_paths_2 = list((ROOT_PATH / "src/icpp/canisters").glob("*"))
    canister_paths = canister_paths_1 + canister_paths_2
    for canister_path in canister_paths:
        test_api_path = canister_path / "test/test_apis.py"
        try:
            typer.echo("--\nStop the local network")
            run_dfx_cmd("stop")

            typer.echo("--\nStart a clean local network")
            run_dfx_cmd("start --clean --background")

            typer.echo("--\nBuild the wasm")
            run_shell_cmd("icpp build-wasm --to-compile all", cwd=canister_path)

            # typer.echo(
            #     "--\ngzip --keep the wasm (keep original for those who deploy unzipped wasm)"
            # )
            # run_shell_cmd("gzip --keep build/*.wasm", cwd=canister_path)

            typer.echo(f"--\nDeploy {canister_path.name}")
            run_dfx_cmd("deploy", cwd=canister_path)

            typer.echo(f"--\nRun pytest on {test_api_path}")
            run_shell_cmd(f"pytest --network=local {test_api_path}", cwd=ROOT_PATH)

            typer.echo("--\nStop the local network")
            run_dfx_cmd("stop")

        except subprocess.CalledProcessError as e:
            typer.echo("--\nSomething did not pass\nStop the local network")
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
