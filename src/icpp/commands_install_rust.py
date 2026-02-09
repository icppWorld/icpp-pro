"""Handles 'icpp install-rust' """

import sys
import platform
import subprocess
import shutil
import typer
from icpp.__main__ import app
from icpp import config_default
from icpp import __version_rust__, __version_ic_wasi_polyfill__, __version_wasi2ic__
from icpp.run_shell_cmd import run_shell_cmd_with_log
from icpp.utils import remove_readonly

OS_SYSTEM = platform.system()
OS_PROCESSOR = platform.processor()

LOG_FILE = config_default.ICPP_LOGS / "install_rust.log"
TIMEOUT_SECONDS = 1000


def install_rustup(nstep: int, num_steps: int) -> None:
    """Installs rustup into user's icpp folder"""
    typer.echo(f"- {nstep}/{num_steps} Installing rustup (be patient...)")

    cmd = (
        f'curl --proto "=https" --tlsv1.2 -sSf https://sh.rustup.rs | '
        f'sh -s -- --no-modify-path -y --default-toolchain="{__version_rust__}" '
    )

    # Note: in config_default.py, we defined the environment variables for
    #       CARGO_TARGET_DIR, CARGO_HOME, RUSTUP_HOME
    #       which ensures that rust is installed in the correct folder (~/.icpp/rust)
    #
    run_shell_cmd_with_log(LOG_FILE, "w", cmd, timeout_seconds=TIMEOUT_SECONDS)


def install_wasm32_wasip1(nstep: int, num_steps: int) -> None:
    """Installs rust wasm32-wasip1 target into user's icpp folder"""
    typer.echo(
        f"- {nstep}/{num_steps} Installing wasm32-wasip1 target for rust compiler"
    )
    cmd = f"{config_default.RUSTUP} target add wasm32-wasip1 "
    run_shell_cmd_with_log(LOG_FILE, "a", cmd, timeout_seconds=TIMEOUT_SECONDS)


def install_wasi2ic(nstep: int, num_steps: int) -> None:
    """Installs wasi2ic into user's icpp folder"""
    typer.echo(f"- {nstep}/{num_steps} Installing wasi2ic {__version_wasi2ic__}")
    # cmd = (
    #     f"{config_default.CARGO} install "
    #     f"--git https://github.com/wasm-forge/wasi2ic "
    #     f"--tag {__version_wasi2ic__} "
    # )
    # run_shell_cmd_with_log(LOG_FILE, "a", cmd, timeout_seconds=TIMEOUT_SECONDS)

    cmd = "git clone https://github.com/wasm-forge/wasi2ic "
    run_shell_cmd_with_log(
        LOG_FILE,
        "a",
        cmd,
        cwd=config_default.RUST_COMPILER_ROOT,
        timeout_seconds=TIMEOUT_SECONDS,
    )

    cmd = f"git switch --detach {__version_wasi2ic__} "
    run_shell_cmd_with_log(
        LOG_FILE,
        "a",
        cmd,
        cwd=config_default.RUST_COMPILER_ROOT / "wasi2ic",
        timeout_seconds=TIMEOUT_SECONDS,
    )

    cmd = (
        f"{config_default.CARGO} install "
        f" --path {config_default.RUST_COMPILER_ROOT / 'wasi2ic'} "
    )
    run_shell_cmd_with_log(
        LOG_FILE,
        "a",
        cmd,
        cwd=config_default.RUST_COMPILER_ROOT / "wasi2ic",
        timeout_seconds=TIMEOUT_SECONDS,
    )


def install_ic_wasi_polyfill(nstep: int, num_steps: int) -> None:
    """Installs ic-wasi-polyfill as a static library into user's icpp folder"""

    typer.echo(
        f"- {nstep}/{num_steps} Installing ic-wasi-polyfill "
        f" {__version_ic_wasi_polyfill__}"
    )

    cmd = "git clone https://github.com/wasm-forge/ic-wasi-polyfill "
    run_shell_cmd_with_log(
        LOG_FILE,
        "a",
        cmd,
        cwd=config_default.RUST_COMPILER_ROOT,
        timeout_seconds=TIMEOUT_SECONDS,
    )

    cmd = f"git switch --detach {__version_ic_wasi_polyfill__} "

    run_shell_cmd_with_log(
        LOG_FILE,
        "a",
        cmd,
        cwd=config_default.RUST_COMPILER_ROOT / "ic-wasi-polyfill",
        timeout_seconds=TIMEOUT_SECONDS,
    )

    cmd = f"{config_default.CARGO} build --release --target wasm32-wasip1 "

    #
    # The 'transient' feature use the transient file system implementation.
    # This works faster but does not take the advantage of keeping the file
    # system's state in stable memory (and the ability to keep FS state
    # between canister upgrades)
    #
    transient_memory = False
    if transient_memory:
        typer.echo("Using transient memory for file storage.")
        cmd += " --features transient "

    run_shell_cmd_with_log(
        LOG_FILE,
        "a",
        cmd,
        cwd=config_default.RUST_COMPILER_ROOT / "ic-wasi-polyfill",
        timeout_seconds=TIMEOUT_SECONDS,
    )


@app.command()
def install_rust() -> None:
    """Installs rust and required dependencies.

    Compiler will be installed in ~/.icpp"""

    typer.echo(f"Installing rust into {config_default.RUST_COMPILER_ROOT}")
    typer.echo(f"Details in {LOG_FILE}")

    try:
        shutil.rmtree(config_default.RUST_COMPILER_ROOT, onerror=remove_readonly)
    except FileNotFoundError:
        pass
    except OSError as e:
        typer.echo(f"Warning: {e.strerror}")

    config_default.RUST_COMPILER_ROOT.mkdir(parents=True, exist_ok=True)
    config_default.ICPP_LOGS.mkdir(parents=True, exist_ok=True)
    # ----------------------------------------------------------------

    num_steps = 4
    nstep = 1
    try:
        install_rustup(nstep, num_steps)
        nstep += 1

        install_wasm32_wasip1(nstep, num_steps)
        nstep += 1

        install_wasi2ic(nstep, num_steps)
        nstep += 1

        install_ic_wasi_polyfill(nstep, num_steps)
        nstep += 1

        typer.echo("\nSuccessfully installed rust & required dependencies ")
        try:
            typer.echo("💯 🎉 🏁")
        except UnicodeEncodeError:
            typer.echo(" ")
        typer.echo("--")

    except subprocess.CalledProcessError as e:
        typer.echo(f"\nError: {e.output}")
        sys.exit(1)
    except Exception as e:  # pylint: disable=broad-except
        typer.echo(f"\nAn unexpected error occurred: {str(e)}")
        sys.exit(1)


def is_rust_installed() -> bool:
    """Returns True if rust and the required dependencies are installed."""
    required_paths = [
        config_default.RUST_COMPILER_ROOT,
        config_default.RUST_BIN,
        config_default.CARGO,
        # config_default.CARGO_BINSTALL, # Not using this yet...
        config_default.RUSTUP,
        config_default.RUSTC,
        config_default.WASI2IC,
        config_default.IC_WASI_POLYFILL,
    ]

    return all(path.exists() for path in required_paths)
