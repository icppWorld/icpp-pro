"""Handles 'icpp install-wasi-sdk' """
import sys
from pathlib import Path
import math
import tarfile
import requests
import enlighten  # type: ignore
import typer
from icpp.__main__ import app
from icpp import config_default


@app.command()
def install_wasi_sdk() -> None:
    """Install wasi-sdk compiler.

    Installs the wasi-sdk compiler for icpp.
    """
    if config_default.WASI_SDK_OS_NAME == "unknown":
        typer.echo("ERROR: a wasi-sdk binary is not available for your system.")
        typer.echo(f"       - Your system: {config_default.OS_SYSTEM}")
        typer.echo(
            f"       Please follow instructions at "
            f"       https://github.com/WebAssembly/wasi-sdk to build it yourself"
            f"       and install it in {config_default.ICPP_ROOT_COMPILER}"
        )
        sys.exit(1)

    try:
        ################################################################################
        # Download the tar.gz file
        use_progress_bar = True

        fpath = Path(str(config_default.ICPP_ROOT_COMPILER) + ".tar.gz")

        with requests.Session() as s:
            r = s.get(config_default.WASI_SDK_URL, stream=use_progress_bar)
            if r.status_code != 200:
                typer.echo(
                    f"Tried to fetch wasi-sdk from GitHub, but response status code is "
                    f"{r.status_code}. Please retry later..."
                )
                typer.echo(f"url = {config_default.WASI_SDK_URL}")
                sys.exit(1)

            typer.echo(f"Downloading wasi-sdk: {config_default.WASI_SDK_URL}")

            config_default.ICPP_ROOT.mkdir(parents=True, exist_ok=True)
            if not use_progress_bar:
                with open(fpath, "wb") as f:
                    f.write(r.content)
            else:
                # https://stackoverflow.com/a/63832993/5480536
                progress_bar_manager = enlighten.get_manager()
                dlen = int(r.headers.get("Content-Length", "0")) or None

                with progress_bar_manager.counter(
                    color="cyan",
                    total=dlen and math.ceil(dlen / 2**20),
                    unit="MiB",
                    leave=False,
                ) as ctr, open(fpath, "wb", buffering=2**24) as f:
                    for chunk in r.iter_content(chunk_size=2**20):
                        # print(chunk[-16:].hex().upper())
                        f.write(chunk)
                        ctr.update()

                progress_bar_manager.stop()

        ################################################################################
        # Unzip the tar.gz file
        typer.echo(f"Installing wasi-sdk in {config_default.ICPP_ROOT_COMPILER}")
        with tarfile.open(fpath, "r") as tar:
            tar.extractall(path=config_default.ICPP_ROOT)

        typer.echo("Cleaning up")
        fpath.unlink(missing_ok=False)

        typer.echo(
            f"Successfully installed wasi-sdk in {config_default.ICPP_ROOT_COMPILER}"
        )
        typer.echo("💯 🎉 🏁")
        typer.echo("--")

    except Exception as e:  # pylint: disable=broad-except
        typer.echo(
            f"Tried to fetch wasi-sdk from GitHub at url={config_default.WASI_SDK_URL} "
            f"and install in {config_default.ICPP_ROOT_COMPILER}, but received this "
            f"Error: {e}"
        )
        sys.exit(1)


def is_wasi_sdk_installed() -> bool:
    """Returns True if the wasi-sdk is installed."""
    if config_default.ICPP_ROOT_COMPILER.exists():
        return True

    return False
