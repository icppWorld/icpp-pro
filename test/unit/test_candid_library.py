"""The icpp-candid library folder: a saved library is reused only for the same
stamp (commands, target, versions), copies are verified, saves are atomic, and
it can never fail a build."""

# pylint: disable=missing-function-docstring, redefined-outer-name

import os
import platform
import re
import shutil
import stat
import subprocess
import threading
from pathlib import Path
from typing import Dict

import pytest
import typer
from typer.testing import CliRunner

from icpp import candid_library
from icpp.__main__ import app
from icpp.options_build import CandidLibraryDirOption

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


def test__the_stamp_follows_commands_sources_target_host_compilers_and_versions(
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
    with monkeypatch.context() as m:
        m.setattr(candid_library, "_compiler_version", lambda command: "other 1.0")
        keys.add(candid_library.stamp(TARGET, commands(), SOURCES)[1])
    with monkeypatch.context() as m:
        m.setattr(platform, "machine", lambda: "other-arch")
        keys.add(candid_library.stamp(TARGET, commands(), SOURCES)[1])
    monkeypatch.setattr(candid_library, "__version__", "0.0.0-test")
    keys.add(candid_library.stamp(TARGET, commands(), SOURCES)[1])
    assert len(keys) == 8


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


def test__parallel_saves_leave_one_entry(
    tmp_path: Path, built: Path, capsys: pytest.CaptureFixture[str]
) -> None:
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
    assert "cannot save" not in capsys.readouterr().out
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


def test__a_relative_folder_is_made_absolute_without_resolving_symlinks(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    (tmp_path / "real").mkdir()
    (tmp_path / "link").symlink_to(tmp_path / "real")
    monkeypatch.chdir(tmp_path / "link")
    assert candid_library.folder("x") == Path(os.getcwd()) / "x"
    assert candid_library.folder("x").is_absolute()  # type: ignore[union-attr]


def test__an_unknown_user_folder_is_off(capsys: pytest.CaptureFixture[str]) -> None:
    assert candid_library.folder("~no-such-user-icpp/x") is None
    assert "not reusing the library" in capsys.readouterr().out


@pytest.mark.parametrize(
    "command", ["build-wasm", "build-native", "build-library", "build-library-native"]
)
def test__the_option_and_its_environment_variable(command: str) -> None:
    """Each build command has --candid-library-dir, also set by the variable."""
    result = CliRunner().invoke(app, [command, "--help"], env={"COLUMNS": "200"})
    assert result.exit_code == 0, result.output
    # CI forces color, which splits the names with ANSI codes
    text = re.sub(r"\x1b\[[0-9;]*m", "", result.output)
    assert "--candid-library-dir" in text
    assert "ICPP_CANDID_LIBRARY_DIR" in text


def test__the_option_wins_over_the_environment_variable(
    monkeypatch: pytest.MonkeyPatch,
) -> None:
    """On a standalone app with the same option: no toolchain, no module state."""
    probe = typer.Typer()
    seen: Dict[str, str | None] = {}

    @probe.command()
    def build(candid_library_dir: CandidLibraryDirOption = None) -> None:
        seen["dir"] = candid_library_dir

    monkeypatch.setenv("ICPP_CANDID_LIBRARY_DIR", "from-env")
    CliRunner().invoke(probe, [])
    assert seen["dir"] == "from-env"
    CliRunner().invoke(probe, ["--candid-library-dir", "from-option"])
    assert seen["dir"] == "from-option"
    monkeypatch.delenv("ICPP_CANDID_LIBRARY_DIR")
    CliRunner().invoke(probe, [])
    assert seen["dir"] is None


# ---------------------------------------------------------------------------
# Trust: a saved library is linked into a canister


def test__the_folder_is_private(tmp_path: Path, built: Path) -> None:
    """Folders 0700, files 0600, whatever the umask."""
    root = tmp_path / "folder"
    old = os.umask(0o022)
    try:
        key = saved(root, built)
    finally:
        os.umask(old)
    entry = root / TARGET / key
    for folder in (root, root / TARGET, entry):
        assert stat.S_IMODE(folder.stat().st_mode) == 0o700, folder
    for file in entry.iterdir():
        assert stat.S_IMODE(file.stat().st_mode) == 0o600, file


@pytest.mark.parametrize("where", ["root", "target", "entry", "library", "stamp"])
def test__a_symlink_is_never_trusted(
    tmp_path: Path, built: Path, where: str, capsys: pytest.CaptureFixture[str]
) -> None:
    """A planted symlink could point at a library someone else controls."""
    root = tmp_path / "folder"
    key = saved(root, built)
    paths = {
        "root": root,
        "target": root / TARGET,
        "entry": root / TARGET / key,
        "library": root / TARGET / key / LIB_FILE,
        "stamp": root / TARGET / key / "stamp.json",
    }
    path = paths[where]
    elsewhere = tmp_path / "elsewhere"
    shutil.move(str(path), elsewhere)
    path.symlink_to(elsewhere)
    other = tmp_path / "other-canister"
    other.mkdir()
    real_root = root if where != "root" else elsewhere
    assert real_root.exists()
    assert not candid_library.restore(root, TARGET, key, LIB_FILE, other)
    assert not (other / LIB_FILE).exists()
    assert "is a symlink" in capsys.readouterr().out


@pytest.mark.parametrize("where", ["entry", "library", "stamp"])
def test__a_saved_copy_writable_by_others_is_never_trusted(
    tmp_path: Path, built: Path, where: str, capsys: pytest.CaptureFixture[str]
) -> None:
    root = tmp_path / "folder"
    key = saved(root, built)
    entry = root / TARGET / key
    path = {"entry": entry, "library": entry / LIB_FILE, "stamp": entry / "stamp.json"}[
        where
    ]
    os.chmod(path, 0o775 if where == "entry" else 0o664)
    assert not candid_library.restore(root, TARGET, key, LIB_FILE, tmp_path)
    assert "is writable by others" in capsys.readouterr().out


def test__open_folders_of_our_own_are_made_private_again(
    tmp_path: Path, built: Path
) -> None:
    """A `tar` restore (actions/cache) creates the folders with the umask, e.g.
    0775: our own folders are made 0700 again, and the library is used."""
    root = tmp_path / "folder"
    key = saved(root, built)
    for folder in (root, root / TARGET):
        os.chmod(folder, 0o775)
    assert candid_library.restore(root, TARGET, key, LIB_FILE, tmp_path)
    for folder in (root, root / TARGET):
        assert stat.S_IMODE(folder.stat().st_mode) == 0o700, folder


def test__a_symlinked_folder_is_not_saved_into(
    tmp_path: Path, built: Path, capsys: pytest.CaptureFixture[str]
) -> None:
    elsewhere = tmp_path / "elsewhere"
    elsewhere.mkdir()
    root = tmp_path / "folder"
    root.symlink_to(elsewhere)
    saved(root, built)
    assert "is a symlink" in capsys.readouterr().out
    assert not any(elsewhere.iterdir())


def test__a_folder_whose_parent_others_can_write_is_refused(
    tmp_path: Path, built: Path, capsys: pytest.CaptureFixture[str]
) -> None:
    """Someone who can write the parent could swap the folder for theirs."""
    parent = tmp_path / "open"
    parent.mkdir()
    os.chmod(parent, 0o777)
    try:
        saved(parent / "folder", built)
        assert "is writable by others" in capsys.readouterr().out
        assert not (parent / "folder" / TARGET).exists()
    finally:
        os.chmod(parent, 0o755)


def test__a_sticky_parent_like_tmp_is_fine(tmp_path: Path, built: Path) -> None:
    """/tmp is writable by all but sticky: nobody can rename our folder."""
    parent = tmp_path / "sticky"
    parent.mkdir()
    os.chmod(parent, 0o1777)
    try:
        root = parent / "folder"
        key = saved(root, built)
        assert candid_library.restore(root, TARGET, key, LIB_FILE, tmp_path)
    finally:
        os.chmod(parent, 0o755)
