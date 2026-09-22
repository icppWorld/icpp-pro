"""Verifies that a canister built with the released icpp-pro upgrades cleanly.

This is the backward-compatibility gate of README-feature-guide.md: a canister
built & deployed with the latest icpp-pro release from PyPI must be upgradable
in place by a build from the current (dev) tree, with all state surviving —
no data-structure migration block, no source changes.

Flow (default canister: ../icpp-demos/canisters/files, whose tests write files
and re-verify them after an upgrade via the `run_after_upgrade` marker):
1. Create a throwaway venv and `pip install icpp-pro` (the released version).
2. Build the canister wasm with the released icpp; deploy fresh on a clean
   project-local network; run its full pytest (this writes state).
3. Rebuild the same source with the dev icpp (this environment); redeploy with
   `--mode upgrade`; run the `run_after_upgrade`-marked tests.

Prerequisites: the contributors-guide setup with the target project's extras
(`make install-python-w-demos` for the default canister,
`make install-python-w-llama_cpp_canister` for `make upgrade-test-llama`),
wasi-sdk, and icp-cli. The pytest phases deliberately run in the DEV
environment - that is the dev side of the gate - while the throwaway venv
only supplies the RELEASED icpp build toolchain (plus the project's own
requirements.txt, for build-time hooks like a post_wasm optimizer). The
deploy identity comes from ${ICPP_PRO_TEST_IDENTITY}; the Makefile exports
it.

Usage:
    make upgrade-test
    python -m scripts.upgrade_test [--canister-dir DIR] [--released-version X.Y.Z]
"""

import argparse
import sys
import tempfile
from pathlib import Path
from typing import List

import typer
from icpp.smoketest import IDENTITY_ENV_VAR

from scripts.all_canister_deploy_local_pytest import (
    TEST_IDENTITY,
    StepError,
    network_start_clean,
    network_stop,
    run_step,
)

ROOT_PATH = Path(__file__).parent.parent.resolve()
DEFAULT_CANISTER_DIR = ROOT_PATH.parent / "icpp-demos/canisters/files"


def parse_args() -> argparse.Namespace:
    """Gets the args"""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--canister-dir",
        default=str(DEFAULT_CANISTER_DIR),
        help="a stateful canister project with run_after_upgrade pytest marks",
    )
    parser.add_argument(
        "--released-version",
        default="",
        help="icpp-pro version to upgrade FROM (default: latest PyPI release)",
    )
    parser.add_argument(
        "--pytest-before",
        default="pytest -vv --network=local test",
        help="pytest command to run against the released build (writes state)",
    )
    parser.add_argument(
        "--pytest-after",
        default="pytest -vv --network=local -m run_after_upgrade test",
        help="pytest command to run after the in-place upgrade",
    )
    return parser.parse_args()


def upgrade_test(
    canister_dir: Path, released_version: str, pytest_before: str, pytest_after: str
) -> int:
    """Deploys with the released icpp-pro, upgrades with the dev tree."""
    log: List[str] = []
    with tempfile.TemporaryDirectory() as tmp:
        venv = Path(tmp) / "venv"
        pip = venv / "bin" / "pip"
        icpp_released = venv / "bin" / "icpp"
        pin = f"icpp-pro=={released_version}" if released_version else "icpp-pro"
        try:
            typer.echo(f"-- install the released icpp-pro ({pin}) in a venv")
            run_step(f'"{sys.executable}" -m venv "{venv}"', canister_dir, log, True)
            run_step(f'"{pip}" install --quiet "{pin}"', canister_dir, log, True)
            # A canister project may need its own deps for the build (e.g. a
            # post_wasm_function's imports). icpp-binaryen, which powers the
            # built-in globals-limit fix, now comes in as an icpp-pro
            # dependency rather than a per-project one. The project's
            # requirements.txt is installed last, so its own icpp-pro pin
            # defines the released baseline being upgraded from.
            if (canister_dir / "requirements.txt").exists():
                typer.echo("-- install the canister project's requirements")
                run_step(
                    f'"{pip}" install --quiet -r requirements.txt',
                    canister_dir,
                    log,
                    True,
                )
            run_step(f'"{icpp_released}" --version', canister_dir, log, True)
            typer.echo("-- the dev icpp-pro it will be upgraded to")
            run_step("icpp --version", canister_dir, log, True)

            typer.echo("-- build with the RELEASED icpp-pro & deploy fresh")
            network_start_clean(canister_dir, log, True)
            run_step(
                f'"{icpp_released}" build-wasm --to-compile all',
                canister_dir,
                log,
                True,
            )
            run_step(
                f"icp deploy --environment local --yes --identity {TEST_IDENTITY}",
                canister_dir,
                log,
                True,
            )
            typer.echo("-- run pytest against the released build")
            run_step(pytest_before, canister_dir, log, True)

            typer.echo("-- rebuild with the DEV icpp-pro & upgrade in place")
            run_step("icpp build-wasm --to-compile all", canister_dir, log, True)
            run_step(
                f"icp deploy --environment local --yes --mode upgrade "
                f"--identity {TEST_IDENTITY}",
                canister_dir,
                log,
                True,
            )
            typer.echo("-- verify behavior after the upgrade")
            run_step(pytest_after, canister_dir, log, True)
        except StepError as e:
            typer.echo(f"❌ upgrade test FAILED at: {e}")
            return 1
        finally:
            network_stop(canister_dir)

    typer.echo("✅ upgrade test passed: released ➜ dev upgrade is clean")
    return 0


def main() -> int:
    """Runs the released ➜ dev upgrade test for one canister."""
    args = parse_args()
    if not TEST_IDENTITY:
        typer.echo(
            f"ERROR: ${IDENTITY_ENV_VAR} is not set.\n"
            f"       It names the identity the canister is deployed with and\n"
            f"       the tests run as. Run this through the Makefile, which\n"
            f"       exports it:\n\n"
            f"         make upgrade-test\n"
        )
        return 1

    canister_dir = Path(args.canister_dir).resolve()
    if not (canister_dir / "icp.yaml").exists():
        typer.echo(f"ERROR: {canister_dir} is not an icp project (no icp.yaml)")
        return 1

    return upgrade_test(
        canister_dir, args.released_version, args.pytest_before, args.pytest_after
    )


if __name__ == "__main__":
    sys.exit(main())
