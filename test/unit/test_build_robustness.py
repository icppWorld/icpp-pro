"""The build commands survive odd file names, bad names in icpp.toml and a
symlinked build folder.

Regressions:
- a file name with a space or a quote split into other words in the shell;
- an empty or path-like `lib_name` deleted the whole `build-library/` folder,
  or one outside it;
- a build folder that could not be deleted was built on top of;
- a project file named like a module icpp imports, eg. `tomllib.py`, was
  imported instead of it.
"""

# pylint: disable=missing-function-docstring, import-outside-toplevel, redefined-outer-name

import importlib
import inspect
import os
import shlex
import sys
from pathlib import Path
from types import ModuleType

import pytest

import icpp

# Imports the build commands in the order the icpp command does
from icpp import __main__ as icpp_main
from icpp import config_default
from icpp.utils import delete_build_folder


def load_icpp_toml(monkeypatch: pytest.MonkeyPatch, toml_text: str) -> ModuleType:
    """Imports icpp_toml for an icpp.toml with this text, in a fresh project."""
    project = Path(os.getcwd())
    (project / "icpp.toml").write_text(toml_text, encoding="utf-8")
    monkeypatch.setattr(config_default, "ICPP_TOML_PATH", project / "icpp.toml")
    # The import sets both the sys.modules entry and the package attribute:
    # set each first, so the teardown restores it or removes it
    monkeypatch.setitem(sys.modules, "icpp.icpp_toml", None)
    monkeypatch.delitem(sys.modules, "icpp.icpp_toml")
    monkeypatch.setattr(icpp, "icpp_toml", None, raising=False)
    monkeypatch.delattr(icpp, "icpp_toml")
    return importlib.import_module("icpp.icpp_toml")


@pytest.fixture
def project(tmp_path: Path, monkeypatch: pytest.MonkeyPatch) -> Path:
    folder = tmp_path / "my project"
    (folder / "src").mkdir(parents=True)
    monkeypatch.chdir(folder)
    return folder


WASM_TABLE = """
[build-wasm]
canister = "my_canister"
did_path = "src/my_canister.did"
cpp_paths = ["src/*.cpp"]
"""


def test__file_names_are_quoted(project: Path, monkeypatch: pytest.MonkeyPatch) -> None:
    cpp_file = project / "src" / "a b'c.cpp"
    cpp_file.write_text("", encoding="utf-8")
    icpp_toml = load_icpp_toml(monkeypatch, WASM_TABLE)
    build_wasm = icpp_toml.build_wasm
    assert shlex.split(build_wasm["cpp_files"]) == [str(cpp_file.resolve())]
    assert shlex.split(build_wasm["cpp_include_flags"]) == [
        "-I",
        str(project.resolve()),
        "-I",
        str((project / "src").resolve()),
    ]


@pytest.mark.parametrize(
    "lib_name", ["", ".", "..", "../x", "a b", "a;b", "-demo", ".hidden"]
)
def test__a_bad_lib_name_exits(
    project: Path, monkeypatch: pytest.MonkeyPatch, lib_name: str
) -> None:
    del project
    toml_text = WASM_TABLE + f'\n[[build-library]]\nlib_name = "{lib_name}"\n'
    with pytest.raises(SystemExit):
        load_icpp_toml(monkeypatch, toml_text)


def test__a_bad_canister_name_exits(
    project: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    del project
    with pytest.raises(SystemExit):
        load_icpp_toml(monkeypatch, WASM_TABLE.replace("my_canister", "../x", 1))


def test__good_names_are_accepted(
    project: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    del project
    toml_text = WASM_TABLE + '\n[[build-library]]\nlib_name = "lib-hello_1.0"\n'
    icpp_toml = load_icpp_toml(monkeypatch, toml_text)
    assert icpp_toml.libraries[0]["lib_name"] == "lib-hello_1.0"


def test__a_symlinked_build_folder_is_refused(tmp_path: Path) -> None:
    target = tmp_path / "elsewhere"
    target.mkdir()
    (target / "keep.txt").write_text("keep", encoding="utf-8")
    build = tmp_path / "build"
    build.symlink_to(target, target_is_directory=True)
    with pytest.raises(SystemExit):
        delete_build_folder(build)
    assert (target / "keep.txt").exists()


def test__a_library_in_a_symlinked_build_root_is_not_deleted(
    tmp_path: Path, project: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    from icpp.commands_build_library import build_library

    target = tmp_path / "elsewhere"
    (target / "libhello").mkdir(parents=True)
    (target / "libhello" / "keep.txt").write_text("keep", encoding="utf-8")
    (project / "build-library").symlink_to(target, target_is_directory=True)
    toml_text = WASM_TABLE + '\n[[build-library]]\nlib_name = "libhello"\n'
    load_icpp_toml(monkeypatch, toml_text)
    with pytest.raises(SystemExit):
        inspect.unwrap(build_library)(lib_name="libhello")
    assert (target / "libhello" / "keep.txt").exists()


def test__a_symlinked_build_folder_is_refused_when_it_is_not_deleted(
    tmp_path: Path,
    project: Path,
    monkeypatch: pytest.MonkeyPatch,
    capsys: pytest.CaptureFixture[str],
) -> None:
    from icpp.commands_build_native import build_native

    target = tmp_path / "elsewhere"
    target.mkdir()
    (project / "build-native").symlink_to(target, target_is_directory=True)
    load_icpp_toml(monkeypatch, WASM_TABLE)
    with pytest.raises(SystemExit):
        inspect.unwrap(build_native)(to_compile="mine-no-lib")
    assert "is a symlink" in capsys.readouterr().out


def test__the_package_attribute_is_restored() -> None:
    # Runs after the tests above: none left its icpp_toml behind
    assert getattr(icpp, "icpp_toml", None) is sys.modules.get("icpp.icpp_toml")


def test__a_build_folder_that_cannot_be_deleted_exits(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    build = tmp_path / "build"
    build.mkdir()

    def fail(_path: Path) -> None:
        raise PermissionError(13, "Permission denied")

    monkeypatch.setattr("icpp.utils.shutil.rmtree", fail)
    with pytest.raises(SystemExit):
        delete_build_folder(build)


def test__a_build_folder_is_deleted(tmp_path: Path) -> None:
    build = tmp_path / "build"
    (build / "sub").mkdir(parents=True)
    delete_build_folder(build)
    assert not build.exists()
    delete_build_folder(build)  # a missing folder is fine


def test__icpp_does_not_put_the_project_folder_on_sys_path(
    project: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    monkeypatch.setattr(sys, "path", list(sys.path))
    monkeypatch.setattr(icpp_main, "app", lambda **_kwargs: None)
    icpp_main.main()
    assert str(project) not in sys.path


def test__a_post_wasm_function_package_wins_over_an_installed_one(
    project: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    from icpp.call_function_from_name import call_function

    # A project package named like a standard library module
    (project / "colorsys").mkdir()
    (project / "colorsys" / "__init__.py").write_text("", encoding="utf-8")
    # The hook imports a top-level module of the project when it runs
    (project / "project_helper.py").write_text("ANSWER = 'yes'\n", "utf-8")
    (project / "colorsys" / "hook.py").write_text(
        "from pathlib import Path\n\n"
        "def run() -> None:\n"
        "    from project_helper import ANSWER\n"
        "    Path('ran.txt').write_text(ANSWER)\n",
        encoding="utf-8",
    )
    monkeypatch.setattr(sys, "path", list(sys.path))
    # Set each first, so the teardown restores it or removes it
    for name in ["colorsys", "colorsys.hook", "project_helper"]:
        monkeypatch.setitem(sys.modules, name, None)
        monkeypatch.delitem(sys.modules, name)
    # The project folder is not on the path, as for the icpp command, so the
    # hook finds its helper only while call_function keeps the folder on it
    sys.path[:] = [p for p in sys.path if p not in ("", ".", str(project))]
    before = list(sys.path)
    call_function("colorsys.hook.run")
    assert (project / "ran.txt").read_text(encoding="utf-8") == "yes"
    assert sys.path == before
