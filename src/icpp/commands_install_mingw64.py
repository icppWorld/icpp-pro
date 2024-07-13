"""Handles 'icpp install-mingw64'"""

import sys
import platform
import shutil
import math
import zipfile
import requests
import enlighten  # type: ignore
import typer
from icpp.__main__ import app
from icpp import config_default
from icpp import __version_mingw64__

OS_SYSTEM = platform.system()
OS_PROCESSOR = platform.processor()

# https://winlibs.com/
# Always select a version for:
# - Win64 - x86_64
# - UCRT runtime
MINGW64_ZIP = (
    "winlibs-x86_64-posix-seh-gcc-13.2.0-llvm-18.1.5-mingw-w64ucrt-11.0.1-r8.zip"
)
MINGW64_URL = f"https://github.com/brechtsanders/winlibs_mingw/releases/download/{__version_mingw64__}/{MINGW64_ZIP}"  # pylint: disable=line-too-long


@app.command()
def install_mingw64() -> None:
    """Installs MinGW-w64 compiler (Windows only).

    Compiler will be installed in ~/.icpp"""

    if OS_SYSTEM != "Windows":
        typer.echo("ERROR: MinGw-w64 is only for Windows, not for your system.")
        sys.exit(1)

    # ----------------------------------------------------------------
    typer.echo(f"Installing MinGW-w64 compiler into {config_default.MINGW64_ROOT}")

    fpath = config_default.MINGW64_COMPILER_ROOT / MINGW64_ZIP
    fpath.unlink(missing_ok=True)

    try:
        shutil.rmtree(config_default.MINGW64_COMPILER_ROOT)
    except FileNotFoundError:
        pass
    except OSError as e:
        typer.echo(f"Warning: {e.strerror}")

    # ----------------------------------------------------------------

    try:
        num_steps = 3
        nstep = 1
        ################################################################################
        # Download the zip file

        # Create the directories if they do not yet exist
        fpath.parent.mkdir(parents=True, exist_ok=True)

        use_progress_bar = True

        with requests.Session() as s:
            r = s.get(MINGW64_URL, stream=use_progress_bar)
            if r.status_code != 200:
                typer.echo(
                    f"Tried to fetch mingw64 zip file from GitHub, "
                    f"but response status code is "
                    f"{r.status_code}. Please retry later..."
                )
                typer.echo(f"url = {MINGW64_URL}")
                sys.exit(1)

            typer.echo(f"- {nstep}/{num_steps} Downloading mingw64: {MINGW64_URL}")
            # typer.echo(f"Saving zip file as  : {fpath}")

            config_default.MINGW64_ROOT.mkdir(parents=True, exist_ok=True)
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

        ################################################################################
        # Unzip the zip file
        nstep += 1
        typer.echo(f"- {nstep}/{num_steps} Unzipping {fpath}")

        if not use_progress_bar:
            with zipfile.ZipFile(fpath, "r") as zip_ref:
                zip_ref.extractall(path=config_default.MINGW64_COMPILER_ROOT)
        else:
            with zipfile.ZipFile(fpath, "r") as zip_ref:
                # All the files in the zipfile
                file_list = zip_ref.namelist()

                # Create a progress bar
                progress_bar = progress_bar_manager.counter(
                    color="cyan", total=len(file_list), leave=False, unit="files"
                )

                for file in file_list:
                    zip_ref.extract(file, config_default.MINGW64_COMPILER_ROOT)
                    progress_bar.update()

                # Close the progress bar
                progress_bar.close()

            # Now we can delete the progress_bar_manager
            progress_bar_manager.stop()

        # Cleaning up
        nstep += 1
        typer.echo(f"- {nstep}/{num_steps} Cleaning up")
        fpath.unlink(missing_ok=True)

        typer.echo(
            f"\nSuccessfully installed MinGW-w64 in "
            f"{config_default.MINGW64_COMPILER_ROOT }"
        )
        try:
            typer.echo("💯 🎉 🏁")
        except UnicodeEncodeError:
            typer.echo(" ")
        typer.echo("--")

    except Exception as e:  # pylint: disable=broad-except
        typer.echo(
            f"Tried to fetch MinGW-w64 from GitHub at url={MINGW64_URL} "
            f"and install in {config_default.MINGW64_COMPILER_ROOT}, but received this "
            f"Error: {e}"
        )
        sys.exit(1)


def is_mingw64_installed() -> bool:
    """Returns True if the MinGW-w64 compiler is installed."""
    if config_default.MINGW64_BIN.exists():
        return True

    return False
