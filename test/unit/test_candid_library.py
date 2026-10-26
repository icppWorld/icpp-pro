"""The icpp-candid library folder: a saved library is reused only for the same
stamp (commands, target, versions), copies are verified, saves are atomic, and
it can never fail a build."""

# pylint: disable=missing-function-docstring, redefined-outer-name

import shutil
import subprocess
import threading
from pathlib import Path
from typing import Dict

import pytest
from typer.testing import CliRunner

from icpp import candid_library
from icpp.__main__ import app

LIB_FILE = "__ic_candid__.a"
TARGET = "native"
SOURCES = ["/src/a.cpp", "/src/b.cpp"]


def commands(flags: str = "-O0") -> Dict[str, str]:
    return {"cpp": f"clang++ -std=c++17 {flags}", "c": "clang", "ar": "ar qc"}


@pytest.fixture
def built(tmp_path: Path) -> Path:
    """A build folder holding a small real archive."""
    build_path = tmp_path / "build"
    build_path.mkdir()
    (tmp_path / "a.cpp").write_text("int a() { return 1; }\n", encoding="utf-8")
    subprocess.run(
        f"clang++ -c {tmp_path / 'a.cpp'} -o a.o && ar qc {LIB_FILE} a.o",
        shell=True,
        check=True,
        cwd=build_path,
    )
    return build_path


def saved(root: Path, build_path: Path, flags: str = "-O0") -> str:
    data, key = candid_library.stamp(TARGET, commands(flags), SOURCES)
    candid_library.save(root, TARGET, key, data, LIB_FILE, build_path)
    return key


def test__the_stamp_follows_commands_sources_target_and_versions(
    monkeypatch: pytest.MonkeyPatch,
) -> None:
    keys = {candid_library.stamp(TARGET, commands(), SOURCES)[1]}
    assert candid_library.stamp(TARGET, commands(), SOURCES)[1] in keys
    keys.add(candid_library.stamp(TARGET, commands("-O2"), SOURCES)[1])
    keys.add(candid_library.stamp("wasm32-wasi", commands(), SOURCES)[1])
    keys.add(
        candid_library.stamp(TARGET, dict(commands(), ar="llvm-ar qc"), SOURCES)[1]
    )
    keys.add(candid_library.stamp(TARGET, commands(), SOURCES + ["/src/c.cpp"])[1])
    monkeypatch.setattr(candid_library, "__version__", "0.0.0-test")
    keys.add(candid_library.stamp(TARGET, commands(), SOURCES)[1])
    assert len(keys) == 6


def test__save_then_restore_gives_the_same_library(
    tmp_path: Path, built: Path, capsys: pytest.CaptureFixture[str]
) -> None:
    root = tmp_path / "folder"
    key = saved(root, built)
    other = tmp_path / "other-canister"
    other.mkdir()
    assert candid_library.restore(root, TARGET, key, LIB_FILE, other)
    assert (other / LIB_FILE).read_bytes() == (built / LIB_FILE).read_bytes()
    assert "Using the saved icpp-candid library" in capsys.readouterr().out


def test__other_flags_are_not_reused(tmp_path: Path, built: Path) -> None:
    root = tmp_path / "folder"
    saved(root, built, "-O0")
    other_key = candid_library.stamp(TARGET, commands("-O2"), SOURCES)[1]
    assert not candid_library.restore(root, TARGET, other_key, LIB_FILE, tmp_path)


def test__nothing_saved_is_no_hit(tmp_path: Path) -> None:
    key = candid_library.stamp(TARGET, commands(), SOURCES)[1]
    assert not candid_library.restore(
        tmp_path / "none", TARGET, key, LIB_FILE, tmp_path
    )


def test__a_damaged_library_is_not_used(
    tmp_path: Path, built: Path, capsys: pytest.CaptureFixture[str]
) -> None:
    root = tmp_path / "folder"
    key = saved(root, built)
    lib = root / TARGET / key / LIB_FILE
    lib.write_bytes(lib.read_bytes()[:10])
    other = tmp_path / "other-canister"
    other.mkdir()
    assert not candid_library.restore(root, TARGET, key, LIB_FILE, other)
    assert not (other / LIB_FILE).exists()
    assert "does not match its sha256" in capsys.readouterr().out


def test__a_failed_copy_leaves_no_partial_library(
    tmp_path: Path, built: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    """The build that follows appends to an existing archive (`ar qc`)."""
    root = tmp_path / "folder"
    key = saved(root, built)

    def failing_copy(_src: Path, dst: Path) -> str:
        Path(dst).write_bytes(b"half a library")
        raise OSError(28, "No space left on device")

    monkeypatch.setattr(candid_library, "_copy_hashed", failing_copy)
    other = tmp_path / "other-canister"
    other.mkdir()
    assert not candid_library.restore(root, TARGET, key, LIB_FILE, other)
    assert not (other / LIB_FILE).exists()


def test__parallel_saves_leave_one_entry(tmp_path: Path, built: Path) -> None:
    root = tmp_path / "folder"
    data, key = candid_library.stamp(TARGET, commands(), SOURCES)
    threads = [
        threading.Thread(
            target=candid_library.save,
            args=(root, TARGET, key, data, LIB_FILE, built),
        )
        for _ in range(8)
    ]
    for t in threads:
        t.start()
    for t in threads:
        t.join()
    assert [p.name for p in (root / TARGET).iterdir()] == [key]
    assert candid_library.restore(root, TARGET, key, LIB_FILE, tmp_path)


def test__a_folder_that_cannot_be_written_never_fails_a_build(
    tmp_path: Path, built: Path, capsys: pytest.CaptureFixture[str]
) -> None:
    root = tmp_path / "folder"
    root.write_text("not a folder", encoding="utf-8")
    saved(root, built)
    assert "cannot save the library" in capsys.readouterr().out


def test__no_folder_means_off(monkeypatch: pytest.MonkeyPatch) -> None:
    monkeypatch.delenv("ICPP_CANDID_LIBRARY_DIR", raising=False)
    assert candid_library.folder(None) is None
    assert candid_library.folder("") is None
    assert candid_library.folder("~/x") == Path("~/x").expanduser()


@pytest.mark.parametrize(
    "command", ["build-wasm", "build-native", "build-library", "build-library-native"]
)
def test__the_option_and_its_environment_variable(command: str) -> None:
    """Each build command has --candid-library-dir, also set by the variable."""
    result = CliRunner().invoke(app, [command, "--help"], env={"COLUMNS": "200"})
    assert result.exit_code == 0, result.output
    assert "--candid-library-dir" in result.output
    assert "ICPP_CANDID_LIBRARY_DIR" in result.output


def test__the_option_wins_over_the_environment_variable(
    monkeypatch: pytest.MonkeyPatch, tmp_path: Path
) -> None:
    """typer fills the option from the variable only when it is not given."""
    seen: Dict[str, str | None] = {}

    def fake_build_library(config: str, candid_library_dir: str | None = None) -> None:
        seen["config"] = config
        seen["dir"] = candid_library_dir

    import icpp.commands_build_wasm as build_wasm_module  # pylint: disable=import-outside-toplevel

    monkeypatch.setattr(build_wasm_module, "build_library", fake_build_library)
    monkeypatch.setenv("ICPP_CANDID_LIBRARY_DIR", str(tmp_path / "from-env"))
    monkeypatch.chdir(tmp_path)
    shutil.copy(
        Path(build_wasm_module.__file__).parent / "canisters/greet/icpp.toml", "."
    )
    for args, expected in (
        (["build-wasm"], str(tmp_path / "from-env")),
        (["build-wasm", "--candid-library-dir", "from-option"], "from-option"),
    ):
        CliRunner().invoke(app, args)
        assert seen.get("dir") == expected, (args, seen)
