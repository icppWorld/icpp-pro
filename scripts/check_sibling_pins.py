"""Checks that version & linter pins are in sync across the icpp repos.

The layout is the one prescribed by README-contributors-guide.md: icpp-demos,
icpp-docs & llama_cpp_canister are cloned as siblings of icpp-pro, and
icpp-candid is nested inside icpp-pro.

Checked:
(1) icpp-pro and icpp-candid share the same version number
(2) icpp-pro's pyproject.toml pins icpp-candid at exactly that version
(3) the sibling repos pin icpp-pro at the current version:
    - icpp-demos/requirements.txt        icpp-pro>=X.Y.Z
    - llama_cpp_canister/requirements.txt icpp-pro==X.Y.Z
    - llama_cpp_canister/docker/docker-compose.yml (both the &icpp anchor and
      the image name literal)
(4) icpp-pro's icpp-binaryen pin matches the icpp-binaryen checkout. Note the
    asymmetry with icpp-candid: icpp-candid is always the SAME version as
    icpp-pro, whereas icpp-binaryen's major component IS the bundled Binaryen
    version (116), so it tracks its own version.py, not icpp-pro's.
(5) black / pylint / mypy pins are identical across icpp-pro (pyproject.toml),
    icpp-demos (requirements.txt) and llama_cpp_canister
    (scripts/requirements.txt)

A failure during a release is informative, not fatal: the release guide bumps
icpp-pro/icpp-candid first, and this checker names every sibling file that
still needs the matching bump.

Usage:
    make check-sibling-pins
    python -m scripts.check_sibling_pins
"""

import re
import sys
from pathlib import Path
from typing import List

import typer

ROOT_PATH = Path(__file__).parent.parent.resolve()
REPOS_PATH = ROOT_PATH.parent

VERSION_PY = ROOT_PATH / "src/icpp/version.py"
CANDID_VERSION_PY = ROOT_PATH / "icpp-candid/src/icpp_candid/version.py"
PYPROJECT = ROOT_PATH / "pyproject.toml"
DEMOS_REQUIREMENTS = REPOS_PATH / "icpp-demos/requirements.txt"
LLAMA_REQUIREMENTS = REPOS_PATH / "llama_cpp_canister/requirements.txt"
LLAMA_SCRIPTS_REQUIREMENTS = REPOS_PATH / "llama_cpp_canister/scripts/requirements.txt"
LLAMA_DOCKER_COMPOSE = REPOS_PATH / "llama_cpp_canister/docker/docker-compose.yml"
BINARYEN_VERSION_PY = REPOS_PATH / "icpp-binaryen/src/icpp_binaryen/version.py"

LINTERS = ["black", "pylint", "mypy"]


def extract(path: Path, pattern: str, failures: List[str]) -> str:
    """Returns the first regex group found in path, or "" after a failure."""
    try:
        text = path.read_text(encoding="utf-8")
    except OSError:
        failures.append(f"cannot read {path}")
        return ""
    match = re.search(pattern, text, flags=re.MULTILINE)
    if match is None:
        failures.append(f"pattern {pattern!r} not found in {path}")
        return ""
    return match.group(1)


def check(label: str, expected: str, actual: str, failures: List[str]) -> None:
    """Records a failure when actual differs from expected (or is missing)."""
    if not actual or not expected:
        return  # extract() already recorded the failure
    if actual == expected:
        typer.echo(f"  ✅ {label}: {actual}")
    else:
        failures.append(f"{label}: expected {expected}, found {actual}")


def main() -> int:
    """Verifies every pin, reports all mismatches, exits non-zero on any."""
    failures: List[str] = []

    version = extract(VERSION_PY, r'__version__ = "([^"]+)"', failures)
    typer.echo(f"icpp-pro version: {version}")

    check(
        "icpp-candid version.py",
        version,
        extract(CANDID_VERSION_PY, r'__version__ = "([^"]+)"', failures),
        failures,
    )
    check(
        "icpp-pro pyproject.toml icpp-candid pin",
        version,
        extract(PYPROJECT, r'"icpp-candid>=([^"]+)"', failures),
        failures,
    )
    # icpp-binaryen is NOT version-locked to icpp-pro - its expected value is
    # its own version.py.
    binaryen_version = extract(
        BINARYEN_VERSION_PY, r'__version__ = "([^"]+)"', failures
    )
    check(
        "icpp-pro pyproject.toml icpp-binaryen pin",
        binaryen_version,
        extract(PYPROJECT, r'"icpp-binaryen>=([^"]+)"', failures),
        failures,
    )

    check(
        "icpp-demos requirements.txt icpp-pro pin",
        version,
        extract(DEMOS_REQUIREMENTS, r"^icpp-pro>=([\w.]+)$", failures),
        failures,
    )
    check(
        "llama_cpp_canister requirements.txt icpp-pro pin",
        version,
        extract(LLAMA_REQUIREMENTS, r"^icpp-pro==([\w.]+)$", failures),
        failures,
    )
    check(
        "llama_cpp_canister docker-compose.yml &icpp anchor",
        version,
        extract(LLAMA_DOCKER_COMPOSE, r'icpp: &icpp "([^"]+)"', failures),
        failures,
    )
    check(
        "llama_cpp_canister docker-compose.yml image name",
        version,
        extract(
            LLAMA_DOCKER_COMPOSE,
            r'&base_name "llama-cpp-canister-build:icpp-([^"]+)"',
            failures,
        ),
        failures,
    )

    for tool in LINTERS:
        reference = extract(PYPROJECT, rf'"{tool}==([^"]+)"', failures)
        check(
            f"icpp-demos {tool} pin",
            reference,
            extract(DEMOS_REQUIREMENTS, rf"^{tool}==([\w.]+)$", failures),
            failures,
        )
        check(
            f"llama_cpp_canister {tool} pin",
            reference,
            extract(LLAMA_SCRIPTS_REQUIREMENTS, rf"^{tool}==([\w.]+)$", failures),
            failures,
        )

    if failures:
        typer.echo("--")
        for failure in failures:
            typer.echo(f"  ❌ {failure}")
        typer.echo("❌ sibling pins are OUT OF SYNC")
        return 1

    typer.echo("✅ all sibling pins are in sync")
    return 0


if __name__ == "__main__":
    sys.exit(main())
