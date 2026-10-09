"""Handles 'icpp install-rust'"""

import sys
import platform
import shutil
import subprocess
import tempfile
from pathlib import Path
import typer
from icpp.__main__ import app
from icpp import config_default
from icpp import __version_rust__, __version_ic_wasi_polyfill__, __version_wasi2ic__
from icpp.run_shell_cmd import run_shell_cmd_with_log
from icpp.utils import rmtree_force

OS_SYSTEM = platform.system()
OS_PROCESSOR = platform.processor()

LOG_FILE = config_default.ICPP_LOGS / "install_rust.log"
RUSTUP_INIT_URL = "https://sh.rustup.rs"
TIMEOUT_SECONDS = 1000

# Both tools are pinned by git commit, but neither commit pins its crates.io
# dependencies, so without these lockfiles the build resolves whatever is newest
# and the wasm hash drifts. Regenerate them when bumping either commit.
CARGO_LOCKS = Path(__file__).parent / "cargo_locks"


def install_rustup(nstep: int, num_steps: int) -> None:
    """Installs rustup into user's icpp folder"""
    typer.echo(f"- {nstep}/{num_steps} Installing rustup (be patient...)")

    # Download first, then run: piped as `curl | sh`, a failed download hands sh
    # an empty script, which exits 0, and the install fails a step later on a
    # rustup that is not there.
    with tempfile.TemporaryDirectory() as tmp_dir:
        rustup_init = Path(tmp_dir) / "rustup-init.sh"
        cmd = (
            f'curl --proto "=https" --tlsv1.2 -sSf --retry 3 '
            f'-o "{rustup_init}" {RUSTUP_INIT_URL} '
        )
        run_shell_cmd_with_log(LOG_FILE, "w", cmd, timeout_seconds=TIMEOUT_SECONDS)

        # Note: in config_default.py, we defined the environment variables for
        #       CARGO_TARGET_DIR, CARGO_HOME, RUSTUP_HOME
        #       which ensures that rust is installed in the correct folder (~/.icpp/rust)
        #
        cmd = (
            f'sh "{rustup_init}" --no-modify-path -y '
            f'--default-toolchain="{__version_rust__}" '
        )
        run_shell_cmd_with_log(LOG_FILE, "a", cmd, timeout_seconds=TIMEOUT_SECONDS)


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

    shutil.copyfile(
        CARGO_LOCKS / "wasi2ic.Cargo.lock",
        config_default.RUST_COMPILER_ROOT / "wasi2ic" / "Cargo.lock",
    )
    cmd = (
        f"{config_default.CARGO} install --locked "
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

    shutil.copyfile(
        CARGO_LOCKS / "ic-wasi-polyfill.Cargo.lock",
        config_default.RUST_COMPILER_ROOT / "ic-wasi-polyfill" / "Cargo.lock",
    )
    cmd = f"{config_default.CARGO} build --locked --release --target wasm32-wasip1 "

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
        rmtree_force(config_default.RUST_COMPILER_ROOT)
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
