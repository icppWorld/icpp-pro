"""icpp install-wasi-sdk and install-rust skip an install that completed.

The download, rustup and cargo steps are replaced by stubs, and the toolchain
folders point into a temporary folder.
"""

# pylint: disable=missing-function-docstring, redefined-outer-name

import tarfile
from pathlib import Path
from typing import List

import pytest

# First, as icpp does: the command modules import each other through it
from icpp import __main__  # pylint: disable=unused-import
from icpp import commands_install_rust, commands_install_wasi_sdk, config_default
from icpp.install_marker import MARKER_NAME

RUST_STEPS = [
    "install_rustup",
    "install_wasm32_wasip1",
    "install_wasi2ic",
    "install_ic_wasi_polyfill",
]


@pytest.fixture
def rust_steps(tmp_path: Path, monkeypatch: pytest.MonkeyPatch) -> List[str]:
    """Points the rust folder into tmp_path, and records the steps that run."""
    monkeypatch.setattr(config_default, "RUST_COMPILER_ROOT", tmp_path / "rust")
    monkeypatch.setattr(config_default, "ICPP_LOGS", tmp_path / "logs")
    calls: List[str] = []
    for step in RUST_STEPS:
        monkeypatch.setattr(
            commands_install_rust,
            step,
            lambda nstep, num_steps, step=step: calls.append(step),
        )
    return calls


def test__rust_is_installed_once(rust_steps: List[str]) -> None:
    commands_install_rust.install_rust()
    assert rust_steps == RUST_STEPS
    marker = config_default.RUST_COMPILER_ROOT / MARKER_NAME
    assert marker.read_text(encoding="utf-8").strip() == (
        commands_install_rust.installed_versions()
    )

    rust_steps.clear()
    commands_install_rust.install_rust()
    assert not rust_steps

    commands_install_rust.install_rust(force=True)
    assert rust_steps == RUST_STEPS


def test__rust_that_failed_is_installed_again(
    rust_steps: List[str], monkeypatch: pytest.MonkeyPatch
) -> None:
    def fail(nstep: int, num_steps: int) -> None:
        raise RuntimeError("cargo build failed")

    with monkeypatch.context() as m:
        m.setattr(commands_install_rust, "install_ic_wasi_polyfill", fail)
        with pytest.raises(SystemExit):
            commands_install_rust.install_rust()
    assert not (config_default.RUST_COMPILER_ROOT / MARKER_NAME).exists()

    rust_steps.clear()
    commands_install_rust.install_rust()
    assert rust_steps == RUST_STEPS


def test__rust_of_other_versions_is_installed_again(rust_steps: List[str]) -> None:
    root = config_default.RUST_COMPILER_ROOT
    root.mkdir(parents=True)
    (root / MARKER_NAME).write_text("rust 1.0.0, ...\n", encoding="utf-8")
    commands_install_rust.install_rust()
    assert rust_steps == RUST_STEPS


@pytest.fixture
def downloads(tmp_path: Path, monkeypatch: pytest.MonkeyPatch) -> List[Path]:
    """Points the wasi-sdk folders into tmp_path; the download writes a small tar."""
    root = tmp_path / "wasi-sdk"
    untar_dir = root / "wasi-sdk-untarred"
    monkeypatch.setattr(config_default, "WASI_SDK_ROOT", root)
    monkeypatch.setattr(
        config_default, "WASI_SDK_COMPILER_ROOT", root / "wasi-sdk-25.0"
    )
    monkeypatch.setattr(commands_install_wasi_sdk, "ICPP_ROOT_UNTAR_DIR", untar_dir)
    monkeypatch.setattr(commands_install_wasi_sdk, "WASI_SDK_OS_NAME", "arm64-macos")
    calls: List[Path] = []

    def download(fpath: Path) -> None:
        calls.append(fpath)
        content = tmp_path / "content"
        (content / untar_dir.name / "bin").mkdir(parents=True, exist_ok=True)
        (content / untar_dir.name / "bin" / "clang").write_text("", encoding="utf-8")
        with tarfile.open(fpath, "w:gz") as tar:
            tar.add(content / untar_dir.name, arcname=untar_dir.name)

    monkeypatch.setattr(
        commands_install_wasi_sdk, "download_wasi_sdk_with_retries", download
    )
    return calls


def test__wasi_sdk_is_installed_once(downloads: List[Path]) -> None:
    commands_install_wasi_sdk.install_wasi_sdk()
    root = config_default.WASI_SDK_COMPILER_ROOT
    assert len(downloads) == 1
    assert (root / "bin" / "clang").exists()
    assert (root / MARKER_NAME).read_text(encoding="utf-8").strip() == (
        commands_install_wasi_sdk.WASI_SDK_URL
    )

    commands_install_wasi_sdk.install_wasi_sdk()
    assert len(downloads) == 1

    commands_install_wasi_sdk.install_wasi_sdk(force=True)
    assert len(downloads) == 2
    assert (root / MARKER_NAME).exists()


def test__wasi_sdk_that_failed_is_installed_again(
    downloads: List[Path], monkeypatch: pytest.MonkeyPatch
) -> None:
    def fail(fpath: Path) -> None:
        raise OSError("connection reset")

    with monkeypatch.context() as m:
        m.setattr(commands_install_wasi_sdk, "download_wasi_sdk_with_retries", fail)
        with pytest.raises(SystemExit):
            commands_install_wasi_sdk.install_wasi_sdk()
    assert not (config_default.WASI_SDK_COMPILER_ROOT / MARKER_NAME).exists()

    commands_install_wasi_sdk.install_wasi_sdk()
    assert len(downloads) == 1


def test__a_folder_without_a_marker_is_installed_again(downloads: List[Path]) -> None:
    # A folder from before the marker, or from an install that was cut off
    (config_default.WASI_SDK_COMPILER_ROOT / "bin").mkdir(parents=True)
    commands_install_wasi_sdk.install_wasi_sdk()
    assert len(downloads) == 1
