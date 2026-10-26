"""Reuses a saved icpp-candid library (__ic_candid__) from a folder that the
user or CI chooses, instead of building it again for every canister.

Turned on by `--candid-library-dir DIR`, or by ICPP_CANDID_LIBRARY_DIR. Off, the
build is exactly as before. With a folder, a build uses the library saved there
for its exact compile & archive commands, its list of source files, the target
and the icpp-pro & icpp-candid versions (the stamp), or builds the library and
saves it there.

Nothing checks the sources or headers: whoever sets the folder guarantees that
they do not change while the folder is in use. `make all-tests` uses a fresh
folder per run; CI keys its cached folder on the hashes of the sources.
"""

import hashlib
import json
import os
import shutil
import uuid
from pathlib import Path
from typing import Any, Dict, Optional, Sequence, Tuple

import typer

import icpp_candid
from icpp import __version__

STAMP = "stamp.json"


def folder(option: Optional[str]) -> Optional[Path]:
    """The folder from the option (which typer also fills from the environment
    variable), or None when reuse is off."""
    return Path(option).expanduser() if option else None


def stamp(
    target: str, commands: Dict[str, str], sources: Sequence[str]
) -> Tuple[Dict[str, Any], str]:
    """What a saved library must match to be reused, and its sha256 (the name of
    the subfolder it is saved in). sources is the list of files it compiles:
    their names, not their content."""
    data = {
        "icpp_pro": __version__,
        "icpp_candid": icpp_candid.__version__,
        "target": target,
        "commands": commands,
        "sources": sorted(str(s) for s in sources),
    }
    return data, hashlib.sha256(json.dumps(data, sort_keys=True).encode()).hexdigest()


def _copy_hashed(src: Path, dst: Path) -> str:
    """Copies src to dst in one pass, and returns the sha256 of what was copied."""
    h = hashlib.sha256()
    with open(src, "rb") as fin, open(dst, "wb") as fout:
        for block in iter(lambda: fin.read(1 << 20), b""):
            h.update(block)
            fout.write(block)
    return h.hexdigest()


def _warn(message: str) -> None:
    typer.echo(f"WARNING: candid library folder: {message}")


def restore(root: Path, target: str, key: str, lib_file: str, build_path: Path) -> bool:
    """Copies the saved library for key into build_path. True when it did; a
    copy that does not match its sha256 is removed, and the library is built."""
    entry = root / target / key
    copy = build_path / lib_file
    try:
        if not (entry / STAMP).is_file() or not (entry / lib_file).is_file():
            return False
        saved = json.loads((entry / STAMP).read_text(encoding="utf-8"))
        if _copy_hashed(entry / lib_file, copy) != saved.get("lib_sha256"):
            copy.unlink()
            _warn(f"{entry / lib_file} does not match its sha256; building it")
            return False
    except Exception as e:  # pylint: disable=broad-except
        try:
            copy.unlink(missing_ok=True)
        except OSError:
            pass
        _warn(f"cannot use {entry}: {e}; building it")
        return False
    typer.echo(f"Using the saved icpp-candid library: {entry}")
    return True


def save(  # pylint: disable=too-many-arguments, too-many-positional-arguments
    root: Path,
    target: str,
    key: str,
    data: Dict[str, Any],
    lib_file: str,
    build_path: Path,
) -> None:
    """Saves the library just built under key. Atomic: written to a temporary
    folder, then renamed, so canisters built in parallel never see a partial
    copy. When another build saved the same key first, that copy is kept."""
    tmp = root / target / f".tmp-{uuid.uuid4().hex}"
    try:
        tmp.mkdir(parents=True)
        lib_sha256 = _copy_hashed(build_path / lib_file, tmp / lib_file)
        (tmp / STAMP).write_text(
            json.dumps(dict(data, lib_sha256=lib_sha256), indent=1, sort_keys=True),
            encoding="utf-8",
        )
        entry = root / target / key
        try:
            os.rename(tmp, entry)
        except OSError:
            if not entry.is_dir():
                raise
            shutil.rmtree(tmp, ignore_errors=True)  # saved by a parallel build
            return
        typer.echo(f"Saved the icpp-candid library: {entry}")
    except Exception as e:  # pylint: disable=broad-except
        shutil.rmtree(tmp, ignore_errors=True)
        _warn(f"cannot save the library in {root}: {e}; the build itself is fine")
