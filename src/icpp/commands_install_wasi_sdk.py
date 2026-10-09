"""Handles 'icpp install-wasi-sdk'"""

import sys
import platform
import shutil
from pathlib import Path
import math
import tarfile
import time
import requests
import enlighten  # type: ignore
import typer
from icpp.__main__ import app
from icpp import config_default
from icpp import __version_wasi_sdk__

# linux, darwin, windows
PLATFORM_SYSTEM = platform.system().lower()

# arm64, aarch64
# x86_64, amd64
PLATFORM_MACHINE = platform.machine().lower()

########################################################
# Convert to naming convention of wasi-sdk

# linux, macos, windows
SYSTEM_NAME = PLATFORM_SYSTEM
if PLATFORM_SYSTEM == "darwin":
    SYSTEM_NAME = "macos"

# arm64 ; x86_64
MACHINE_NAME = "unknown"
if PLATFORM_MACHINE in ["arm64", "aarch64"]:
    MACHINE_NAME = "arm64"
if PLATFORM_MACHINE in ["x86_64", "amd64"]:
    MACHINE_NAME = "x86_64"

########################################################


def get_wasi_sdk_os_name() -> str:
    """Returns the os name used for the releases build by the CI/CD of the wasi-sdk
    repo, for the current machine & system."""

    # wasi-sdk now supports arm64 (Apple Silicon (M1, M2, ...))
    # https://github.com/WebAssembly/wasi-sdk/releases/tag/wasi-sdk-25
    if (MACHINE_NAME in ["arm64", "x86_64"]) and (SYSTEM_NAME in ["linux", "macos"]):
        return f"{MACHINE_NAME}-{SYSTEM_NAME}"

    return "unknown"


def get_wasi_sdk_untar_dir_name() -> str:
    """Returns the dir name after untarring, for the current OS."""
    return f"{__version_wasi_sdk__}-{MACHINE_NAME}-{SYSTEM_NAME}"


WASI_SDK_OS_NAME = get_wasi_sdk_os_name()
WASI_SDK_URL = (
    f"https://github.com/WebAssembly/wasi-sdk/releases/download/"
    f"{__version_wasi_sdk__.split('.',1)[0]}/"
    f"{__version_wasi_sdk__}-{WASI_SDK_OS_NAME}.tar.gz"
)


# Retries for a transient network error, such as a connection that drops
# mid-download. The first pause is RETRY_PAUSE_SECONDS; it doubles every attempt.
DOWNLOAD_RETRIES = 3
RETRY_PAUSE_SECONDS = 5

ICPP_ROOT_UNTAR_DIR = config_default.WASI_SDK_ROOT / f"{get_wasi_sdk_untar_dir_name()}"


def download_wasi_sdk(fpath: Path) -> None:
    """Downloads the wasi-sdk tar ball into fpath, overwriting a partial one.

    Raises a requests.RequestException on a network error or an HTTP error
    status, including one that drops the connection mid-download.
    """
    with requests.Session() as s:
        r = s.get(WASI_SDK_URL, stream=True, timeout=60)
        r.raise_for_status()

        # https://stackoverflow.com/a/63832993/5480536
        progress_bar_manager = enlighten.get_manager()
        dlen = int(r.headers.get("Content-Length", "0")) or None

        with (
            progress_bar_manager.counter(
                color="cyan",
                total=dlen and math.ceil(dlen / 2**20),
                unit="MiB",
                leave=False,
            ) as ctr,
            open(fpath, "wb", buffering=2**24) as f,
        ):
            for chunk in r.iter_content(chunk_size=2**20):
                f.write(chunk)
                ctr.update()

        progress_bar_manager.stop()


def download_wasi_sdk_with_retries(fpath: Path) -> None:
    """Downloads the wasi-sdk tar ball, retrying a transient network error.

    A client error such as a 404 is permanent and raises right away.
    """
    for attempt in range(DOWNLOAD_RETRIES + 1):
        try:
            download_wasi_sdk(fpath)
            return
        except requests.RequestException as e:
            status = e.response.status_code if e.response is not None else None
            permanent = status is not None and 400 <= status < 500 and status != 429
            if permanent or attempt == DOWNLOAD_RETRIES:
                raise
            pause = RETRY_PAUSE_SECONDS * 2**attempt
            typer.echo(f"  Download failed ({e}), retrying in {pause} s...")
            time.sleep(pause)


@app.command()
def install_wasi_sdk() -> None:
    """Installs wasi-sdk compiler.

    Compiler will be installed in ~/.icpp"""

    if WASI_SDK_OS_NAME == "unknown":
        typer.echo("ERROR: a wasi-sdk binary is not available for your system.")
        typer.echo(f"       - Your system: {PLATFORM_SYSTEM}")
        typer.echo(f"       - Your CPU   : {PLATFORM_MACHINE}")
        typer.echo(
            f"       Please follow instructions at "
            f"       https://github.com/WebAssembly/wasi-sdk to build it yourself"
            f"       and install it in {config_default.WASI_SDK_COMPILER_ROOT}"
        )
        sys.exit(1)

    # ----------------------------------------------------------------
    typer.echo(f"Installing wasi-sdk into {config_default.WASI_SDK_ROOT}")

    typer.echo(f"- Your system: {PLATFORM_SYSTEM}")
    typer.echo(f"- Your CPU   : {PLATFORM_MACHINE}")

    fpath = Path(str(config_default.WASI_SDK_COMPILER_ROOT) + ".tar.gz")
    fpath.unlink(missing_ok=True)

    try:
        shutil.rmtree(config_default.WASI_SDK_COMPILER_ROOT)
    except FileNotFoundError:
        pass
    except OSError as e:
        typer.echo(f"Warning: {e.strerror}")

    try:
        shutil.rmtree(ICPP_ROOT_UNTAR_DIR)
    except FileNotFoundError:
        pass
    except OSError as e:
        typer.echo(f"Warning: {e.strerror}")
    # ----------------------------------------------------------------

    try:
        num_steps = 3
        if ICPP_ROOT_UNTAR_DIR != config_default.WASI_SDK_COMPILER_ROOT:
            num_steps = 4  # rename is needed

        nstep = 1
        ################################################################################
        # Download the tar.gz file
        typer.echo(f"- {nstep}/{num_steps} Downloading wasi-sdk: {WASI_SDK_URL}")
        config_default.WASI_SDK_ROOT.mkdir(parents=True, exist_ok=True)
        download_wasi_sdk_with_retries(fpath)

        ################################################################################
        # Unzip the tar.gz file
        nstep += 1
        typer.echo(f"- {nstep}/{num_steps} Untarring {fpath}")
        with tarfile.open(fpath, "r") as tar:
            tar.extractall(path=config_default.WASI_SDK_ROOT)

        if ICPP_ROOT_UNTAR_DIR != config_default.WASI_SDK_COMPILER_ROOT:
            nstep += 1
            typer.echo(
                f"- {nstep}/{num_steps} Renaming {ICPP_ROOT_UNTAR_DIR} to "
                f"{config_default.WASI_SDK_COMPILER_ROOT}"
            )
            try:
                # Convert Path to str, for compatibility with Python 3.8
                shutil.move(
                    str(ICPP_ROOT_UNTAR_DIR),
                    str(config_default.WASI_SDK_COMPILER_ROOT),
                )
            except shutil.Error as e:
                typer.echo(f"Error: {e}")
                sys.exit(1)

        nstep += 1
        typer.echo(f"- {nstep}/{num_steps} Cleaning up")
        fpath.unlink(missing_ok=True)

        typer.echo(
            f"\nSuccessfully installed wasi-sdk in "
            f"{config_default.WASI_SDK_COMPILER_ROOT}"
        )
        try:
            typer.echo("💯 🎉 🏁")
        except UnicodeEncodeError:
            typer.echo(" ")
        typer.echo("--")

    except Exception as e:  # pylint: disable=broad-except
        typer.echo(
            f"Tried to fetch wasi-sdk from GitHub at url={WASI_SDK_URL} "
            f"and install in {config_default.WASI_SDK_COMPILER_ROOT}, but received this "
            f"Error: {e}"
        )
        sys.exit(1)


def is_wasi_sdk_installed() -> bool:
    """Returns True if the wasi-sdk is installed."""
    if config_default.WASI_SDK_COMPILER_ROOT.exists():
        return True

    return False
