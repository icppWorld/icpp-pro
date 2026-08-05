"""Deploys & tests all canisters, each in its own local network.

icp-cli runs a local network per project, and every canister's `icp.yaml` asks
for an ephemeral gateway port (`gateway.port: 0`), so the canisters do not
collide and can be built, deployed and tested concurrently. Under dfx this was
impossible: there was a single global replica on a fixed port.

The default number of jobs is deliberately conservative - `icpp build-wasm` is
itself multi-threaded, so each canister already uses the available cores. On a
CI runner (3-4 vCPU) this resolves to 1 job, i.e. today's serial behaviour; on
a developer machine with many cores it runs several canisters at once. Override
with `--jobs N`.
"""

import argparse
import os
import shutil
import subprocess
import sys
import threading
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path
from typing import List, Tuple

import typer
from icpp.run_shell_cmd import run_shell_cmd
from icpp.smoketest import IDENTITY_ENV_VAR

SCRIPTS_PATH = Path(__file__).parent
ROOT_PATH = Path(__file__).parent.parent

# The canisters are deployed with the identity the tests then run as, so that
# the caller of a test IS the controller. It has to be named explicitly: the
# machine-wide active identity is shared with every other process, which is
# free to change it while a run is in flight. The Makefile exports it.
TEST_IDENTITY = os.environ.get(IDENTITY_ENV_VAR, "")

# Generous ceiling: a cold `icpp build-wasm` compiles the whole C++ tree.
# It matters that this is explicit - `run_shell_cmd` defaults to 30s whenever
# output is captured, which a build blows through.
TIMEOUT_SECONDS = 3600


class StepError(Exception):
    """A step of a canister's pipeline failed."""


# Canister logs are buffered & printed as one block, so concurrent runs stay
# readable. That means the only live progress signal is these one-line
# notices - keep them serialised so they do not interleave mid-line.
PRINT_LOCK = threading.Lock()


def notify(msg: str) -> None:
    """Prints a single progress line, without interleaving across threads."""
    with PRINT_LOCK:
        typer.echo(msg)


def default_jobs(n_canisters: int) -> int:
    """How many canisters to run concurrently, by default.

    `icpp build-wasm` already parallelises across compile units, so running one
    canister per core would oversubscribe. A quarter of the cores keeps CI
    (3-4 vCPU -> 1 job, unchanged behaviour) safe while still giving a
    developer machine real concurrency.
    """
    return max(1, min(n_canisters, (os.cpu_count() or 1) // 4))


def run_step(cmd: str, cwd: Path, log: List[str], stream: bool = False) -> None:
    """Runs one command, appending its output to `log`. Raises on failure.

    With `stream=True` the output goes straight to the terminal as it is
    produced, which is what you want when only one canister is running: CI
    logs stay live (so a hang shows where it stalled) and an interactive
    `--jobs 1` debug session behaves as it always has.

    With `stream=False` the output is captured and returned via `log`, so that
    concurrent canisters do not interleave into an unreadable mess.
    """
    log.append(f"$ {cmd}")
    if stream:
        # No capture => no timeout => output goes straight to the terminal.
        try:
            run_shell_cmd(cmd, cwd=cwd)
        except subprocess.CalledProcessError as e:
            raise StepError(cmd) from e
        return

    try:
        out = run_shell_cmd(
            cmd, capture_output=True, cwd=cwd, timeout_seconds=TIMEOUT_SECONDS
        )
    except subprocess.CalledProcessError as e:
        log.append(e.output or "")
        raise StepError(cmd) from e

    log.append(out)
    # A timeout does NOT raise - run_shell_cmd returns this marker string
    # instead, which would otherwise read as success.
    if out.startswith("ERROR: Command") and "timed out" in out:
        raise StepError(cmd)


def network_stop(canister_path: Path) -> None:
    """Stops the canister's project-local network, if it is running.

    `icp network stop` exits non-zero when no network is running, which is not
    an error for us - we only want the network down afterwards.
    """
    try:
        run_shell_cmd(
            "icp network stop",
            capture_output=True,
            cwd=canister_path,
            timeout_seconds=TIMEOUT_SECONDS,
        )
    except subprocess.CalledProcessError:
        pass


def network_start_clean(canister_path: Path, log: List[str], stream: bool) -> None:
    """Starts a clean, project-local network for the canister.

    icp-cli has no `--clean` flag. A managed network keeps both its replica
    state and its canister id mappings under `.icp/cache`, so removing that
    directory is the equivalent of `dfx start --clean`.

    NOTE: only `.icp/cache` - never `.icp`, which also holds `data/mappings/`
    with the mainnet canister ids.
    """
    network_stop(canister_path)
    shutil.rmtree(canister_path / ".icp" / "cache", ignore_errors=True)
    run_step("icp network start --background", canister_path, log, stream)


def test_canister(
    canister_path: Path, stream: bool = False
) -> Tuple[str, bool, List[str]]:
    """Builds, deploys & tests one canister. Returns (name, ok, log)."""
    name = canister_path.name
    notify(f">>>> {name}: started")
    log: List[str] = [f"==== {name}"]
    test_api_path = canister_path / "test/test_apis.py"
    configs = [file.name for file in canister_path.glob("*.toml")]

    try:
        for config in configs:
            log.append(f"-- start a clean local network ({name})")
            network_start_clean(canister_path, log, stream)

            log.append(f"-- build the wasm with config {config}")
            run_step(
                f"icpp build-wasm --config {config} --to-compile all",
                canister_path,
                log,
                stream,
            )

            log.append(f"-- deploy {name}")
            run_step(
                f"icp deploy --environment local --yes --identity {TEST_IDENTITY}",
                canister_path,
                log,
                stream,
            )

            # pytest runs from the canister directory: that is the icp project
            # root, which is how icp finds icp.yaml and this canister's network.
            log.append(f"-- pytest {test_api_path}")
            run_step(
                f"pytest -vv --network=local {test_api_path}",
                canister_path,
                log,
                stream,
            )

            network_stop(canister_path)

            # For the greet canister, also exercise build-library & --config
            if name == "greet":
                log.append(f"-- start a clean local network ({name}, libraries)")
                network_start_clean(canister_path, log, stream)

                log.append(f"-- build all libraries with config {config}")
                run_step(
                    f"icpp build-library --config {config}", canister_path, log, stream
                )

                log.append(f"-- build libhello with config {config}")
                run_step(
                    f"icpp build-library --config {config} libhello",
                    canister_path,
                    log,
                    stream,
                )

                log.append(f"-- build the wasm with config {config} (mine-no-lib)")
                run_step(
                    f"icpp build-wasm --config {config} --to-compile mine-no-lib",
                    canister_path,
                    log,
                    stream,
                )

                log.append(f"-- deploy {name}")
                run_step(
                    f"icp deploy --environment local --yes --identity {TEST_IDENTITY}",
                    canister_path,
                    log,
                    stream,
                )

                log.append(f"-- pytest {test_api_path}")
                run_step(
                    f"pytest -vv --network=local {test_api_path}",
                    canister_path,
                    log,
                    stream,
                )

                network_stop(canister_path)
        return name, True, log

    except StepError as e:
        log.append(f"!! FAILED: {e}")
        return name, False, log
    finally:
        # Never leave a replica running, whatever happened above.
        network_stop(canister_path)


def main() -> int:
    """Build, deploy & pytest every canister."""
    if not TEST_IDENTITY:
        typer.echo(
            f"ERROR: ${IDENTITY_ENV_VAR} is not set.\n"
            f"       It names the identity the canisters are deployed with and\n"
            f"       the tests run as. Run this through the Makefile, which\n"
            f"       exports it:\n\n"
            f"         make all-canister-deploy-local-pytest\n"
        )
        return 1

    canister_paths = sorted(
        p
        for p in list((ROOT_PATH / "src/icpp/canisters").glob("*"))
        + list((ROOT_PATH / "test/canisters").glob("canister_*"))
        if (p / "icp.yaml").exists()  # skips __pycache__ & friends
    )

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--jobs",
        type=int,
        default=default_jobs(len(canister_paths)),
        help="how many canisters to build & test concurrently (1 = serial)",
    )
    args = parser.parse_args()
    jobs = max(1, args.jobs)

    typer.echo(
        f"Testing {len(canister_paths)} canisters with {jobs} job(s) "
        f"on {os.cpu_count()} core(s): "
        f"{', '.join(p.name for p in canister_paths)}"
    )

    failed: List[str] = []
    done = 0
    with ThreadPoolExecutor(max_workers=jobs) as pool:
        # Stream output when there is no concurrency to garble it: keeps CI
        # logs live and `--jobs 1` debugging exactly as it was before.
        stream = jobs == 1
        futures = [pool.submit(test_canister, p, stream) for p in canister_paths]
        # as_completed, not map: report each canister the moment IT finishes,
        # rather than waiting for the ones submitted before it.
        for future in as_completed(futures):
            name, ok, log = future.result()
            done += 1
            with PRINT_LOCK:
                typer.echo("\n".join(log))
                typer.echo(
                    f"---- {name}: {'PASSED' if ok else 'FAILED'} "
                    f"({done}/{len(canister_paths)})\n"
                )
            if not ok:
                failed.append(name)

    if failed:
        typer.echo(f"--\nSomething did not pass: {', '.join(failed)}")
        return 1

    typer.echo("--\nCongratulations, everything passed!")
    try:
        typer.echo("💯 🎉 🏁")
    except UnicodeEncodeError:
        typer.echo(" ")
    return 0


if __name__ == "__main__":
    sys.exit(main())
