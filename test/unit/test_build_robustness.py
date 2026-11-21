"""The build commands survive odd file names, bad names in icpp.toml and a
symlinked build folder.

Regressions:
- a file name with a space or a quote split into other words in the shell;
- an empty or path-like `lib_name` deleted the whole `build-library/` folder,
  or one outside it;
- a build folder that could not be deleted was built on top of;
- a project file named like a module icpp imports, eg. `tomllib.py`, was
  imported instead of it;
- `--to-compile mine-no-lib` linked the libraries an older icpp built;
- a native library did not get the compile flags of its [[build-library]].
"""

# pylint: disable=missing-function-docstring, import-outside-toplevel, redefined-outer-name

import importlib
import inspect
import os
import shlex
import subprocess
import sys
import threading
from pathlib import Path
from types import ModuleType
from typing import Any, Dict, Optional

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


def test__a_dangling_symlink_as_library_folder_is_refused(
    tmp_path: Path,
    project: Path,
    monkeypatch: pytest.MonkeyPatch,
    capsys: pytest.CaptureFixture[str],
) -> None:
    from icpp.commands_build_library import build_library

    (project / "build-library").mkdir()
    (project / "build-library" / "libhello").symlink_to(tmp_path / "missing")
    toml_text = WASM_TABLE + '\n[[build-library]]\nlib_name = "libhello"\n'
    load_icpp_toml(monkeypatch, toml_text)
    with pytest.raises(SystemExit):
        inspect.unwrap(build_library)(lib_name="libhello")
    assert "is a symlink" in capsys.readouterr().out


class LibrariesBuilt(Exception):
    """Stops the build once it decided to build the libraries."""


def built(*_a: Any, **_k: Any) -> None:
    """Stands in for a library builder."""
    raise LibrariesBuilt


@pytest.mark.parametrize(
    "module, builder, folder",
    [
        ("build_wasm", "build_library", "build"),
        ("build_native", "build_library_native", "build-native"),
    ],
)
def test__a_symlinked_build_folder_is_refused_when_it_is_not_deleted(
    tmp_path: Path,
    project: Path,
    monkeypatch: pytest.MonkeyPatch,
    capsys: pytest.CaptureFixture[str],
    module: str,
    builder: str,
    folder: str,
) -> None:
    target = tmp_path / "elsewhere"
    target.mkdir()
    (project / folder).symlink_to(target, target_is_directory=True)
    load_icpp_toml(monkeypatch, WASM_TABLE)

    # Refused before the libraries, which have no stamp, are built again
    command = importlib.import_module(f"icpp.commands_{module}")
    monkeypatch.setattr(command, builder, built)
    with pytest.raises(SystemExit):
        inspect.unwrap(getattr(command, module))(to_compile="mine-no-lib")
    assert "is a symlink" in capsys.readouterr().out


def stub_didc(monkeypatch: pytest.MonkeyPatch) -> None:
    """didc is found, and binds without running."""
    monkeypatch.setattr("icpp.commands_build_wasm.shutil.which", lambda _: "didc")
    monkeypatch.setattr(
        "icpp.commands_build_wasm.run_shell_cmd", lambda *_a, **_k: "bindings"
    )


@pytest.mark.parametrize(
    "link", ["src", "declarations", "my_canister", "my_canister.did.js"]
)
def test__bindings_are_not_written_through_a_symlink(
    tmp_path: Path,
    project: Path,
    monkeypatch: pytest.MonkeyPatch,
    capsys: pytest.CaptureFixture[str],
    link: str,
) -> None:
    from icpp.commands_build_wasm import generate_javascript_bindings

    target = tmp_path / "elsewhere"
    target.mkdir()
    (target / "keep.txt").write_text("keep", encoding="utf-8")
    declarations = project / "src" / "declarations"
    did_path = project / "my_canister.did"
    did_path.write_text("service : {}", encoding="utf-8")
    if link == "src":
        (project / "src").rmdir()
    paths = {
        "src": project / "src",
        "declarations": declarations,
        "my_canister": declarations / "my_canister",
        "my_canister.did.js": declarations / "my_canister" / "my_canister.did.js",
    }
    paths[link].parent.mkdir(parents=True, exist_ok=True)
    paths[link].symlink_to(target / "keep.txt" if link.endswith(".js") else target)
    stub_didc(monkeypatch)
    with pytest.raises(SystemExit):
        generate_javascript_bindings(did_path, "my_canister", declarations)
    assert f"{paths[link]} is a symlink" in capsys.readouterr().out
    assert sorted(p.name for p in target.iterdir()) == ["keep.txt"]
    assert (target / "keep.txt").read_text(encoding="utf-8") == "keep"


def test__the_did_is_not_copied_into_a_directory_at_its_path(
    tmp_path: Path,
    project: Path,
    monkeypatch: pytest.MonkeyPatch,
    capsys: pytest.CaptureFixture[str],
) -> None:
    from icpp.commands_build_wasm import generate_javascript_bindings

    # A directory named like the .did output, holding a symlink named like the
    # .did input: a copy into the directory would write through the symlink
    target = tmp_path / "keep.txt"
    target.write_text("keep", encoding="utf-8")
    did_path = project / "my_canister.did"
    did_path.write_text("service : {}", encoding="utf-8")
    declarations = project / "src" / "declarations"
    did_folder = declarations / "my_canister" / "my_canister.did"
    did_folder.mkdir(parents=True)
    (did_folder / "my_canister.did").symlink_to(target)
    stub_didc(monkeypatch)
    with pytest.raises(SystemExit):
        generate_javascript_bindings(did_path, "my_canister", declarations)
    assert "failed to write the bindings" in capsys.readouterr().out
    assert target.read_text(encoding="utf-8") == "keep"


def test__the_did_keeps_its_permissions(
    project: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    from icpp.commands_build_wasm import generate_javascript_bindings

    did_path = project / "my_canister.did"
    did_path.write_text("service : {}", encoding="utf-8")
    did_path.chmod(0o600)
    declarations = project / "src" / "declarations"
    stub_didc(monkeypatch)
    generate_javascript_bindings(did_path, "my_canister", declarations)
    out = declarations / "my_canister" / "my_canister.did"
    assert out.read_text(encoding="utf-8") == "service : {}"
    assert out.stat().st_mode & 0o777 == 0o600


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


# ---------------------------------------------------------------------------
# The libraries that `--to-compile mine-no-lib` reuses


def stub_compiles(monkeypatch: pytest.MonkeyPatch, module: str) -> list[str]:
    """The compiles of a library build only record their compile commands."""
    commands: list[str] = []

    def record(fn: Any, files: list[str]) -> None:
        del files
        commands.append(
            fn.keywords.get("cpp_compile_cmd", fn.keywords.get("c_compile_cmd"))
        )

    monkeypatch.setattr(f"icpp.{module}.run_in_parallel", record)
    monkeypatch.setattr(f"icpp.{module}.run_shell_cmd", lambda *_a, **_k: "")
    return commands


def stamp_of(project: Path, root: str, lib_name: str) -> Path:
    """The icpp version stamp in a library folder."""
    from icpp import library_stamp

    return project / root / lib_name / library_stamp.STAMP_NAME


@pytest.mark.parametrize(
    "module, root",
    [
        ("build_library", "build-library"),
        ("build_library_native", "build-library-native"),
    ],
)
def test__a_full_library_build_stamps_each_library(
    project: Path, monkeypatch: pytest.MonkeyPatch, module: str, root: str
) -> None:
    (project / "src" / "hello.cpp").write_text("", encoding="utf-8")
    toml_text = WASM_TABLE + (
        '\n[[build-library]]\nlib_name = "libhello"\ncpp_paths = ["src/hello.cpp"]\n'
    )
    load_icpp_toml(monkeypatch, toml_text)
    stub_compiles(monkeypatch, f"commands_{module}")
    command = importlib.import_module(f"icpp.commands_{module}")
    inspect.unwrap(getattr(command, module))()
    for lib_name in ["libhello", "__ic_candid__"]:
        stamp = stamp_of(project, root, lib_name)
        assert stamp.read_text(encoding="utf-8") == icpp.__version__


@pytest.mark.parametrize(
    "module, root",
    [
        ("build_library", "build-library"),
        ("build_library_native", "build-library-native"),
    ],
)
def test__a_failed_library_build_leaves_no_stamp(
    project: Path, monkeypatch: pytest.MonkeyPatch, module: str, root: str
) -> None:
    stamp = stamp_of(project, root, "__ic_candid__")
    stamp.parent.mkdir(parents=True)
    stamp.write_text(icpp.__version__, encoding="utf-8")
    load_icpp_toml(monkeypatch, WASM_TABLE)
    stub_compiles(monkeypatch, f"commands_{module}")

    def fail(*_a: Any, **_k: Any) -> str:
        raise subprocess.CalledProcessError(1, "ar")

    monkeypatch.setattr(f"icpp.commands_{module}.run_shell_cmd", fail)
    command = importlib.import_module(f"icpp.commands_{module}")
    with pytest.raises(SystemExit):
        inspect.unwrap(getattr(command, module))()
    assert not stamp.exists()


@pytest.mark.parametrize(
    "module, root",
    [
        ("build_library", "build-library"),
        ("build_library_native", "build-library-native"),
    ],
)
def test__a_single_library_build_stamps_only_its_library(
    project: Path, monkeypatch: pytest.MonkeyPatch, module: str, root: str
) -> None:
    toml_text = WASM_TABLE + '\n[[build-library]]\nlib_name = "libhello"\n'
    load_icpp_toml(monkeypatch, toml_text)
    stub_compiles(monkeypatch, f"commands_{module}")
    for lib_name in ["libhello", "__ic_candid__"]:
        stamp_of(project, root, lib_name).parent.mkdir(parents=True)
        stamp_of(project, root, lib_name).write_text("0.0.0", encoding="utf-8")
    command = importlib.import_module(f"icpp.commands_{module}")
    inspect.unwrap(getattr(command, module))(lib_name="libhello")
    assert stamp_of(project, root, "libhello").read_text("utf-8") == icpp.__version__
    assert stamp_of(project, root, "__ic_candid__").read_text("utf-8") == "0.0.0"


@pytest.mark.parametrize("linked", ["build-library", "libhello", "libhello.a", "stamp"])
def test__mine_no_lib_does_not_reuse_a_symlinked_folder(
    tmp_path: Path, capsys: pytest.CaptureFixture[str], linked: str
) -> None:
    from icpp import library_stamp

    outside = tmp_path / "outside"
    (outside / "libhello").mkdir(parents=True)
    (outside / "libhello" / "libhello.a").write_text("", encoding="utf-8")
    stamp = outside / "libhello" / library_stamp.STAMP_NAME
    stamp.write_text(icpp.__version__, "utf-8")
    build_root = tmp_path / "build-library"
    if linked == "build-library":
        build_root.symlink_to(outside)
    elif linked == "libhello":
        build_root.mkdir()
        (build_root / "libhello").symlink_to(outside / "libhello")
    else:
        # A real folder, with the archive or the stamp a symlink
        (build_root / "libhello").mkdir(parents=True)
        name = "libhello.a" if linked == "libhello.a" else library_stamp.STAMP_NAME
        other = "libhello.a" if name != "libhello.a" else library_stamp.STAMP_NAME
        (build_root / "libhello" / name).symlink_to(outside / "libhello" / name)
        (build_root / "libhello" / other).write_text(icpp.__version__, "utf-8")
    libraries = [{"lib_name": "libhello"}]
    assert library_stamp.must_rebuild(build_root, libraries, ".a")
    assert "is a symlink" in capsys.readouterr().out


@pytest.mark.skipif(not hasattr(os, "mkfifo"), reason="no named pipes")
def test__a_stamp_that_is_a_pipe_is_not_read(tmp_path: Path) -> None:
    from icpp import library_stamp

    (tmp_path / "libhello").mkdir()
    (tmp_path / "libhello" / "libhello.a").write_text("", encoding="utf-8")
    os.mkfifo(tmp_path / "libhello" / library_stamp.STAMP_NAME)
    result: list[bool] = []
    checker = threading.Thread(
        target=lambda: result.append(
            library_stamp.must_rebuild(tmp_path, [{"lib_name": "libhello"}], ".a")
        ),
        daemon=True,
    )
    checker.start()
    # A read of the pipe would wait for a writer that never comes
    checker.join(timeout=10)
    assert result == [True]


def test__a_stamp_that_cannot_be_written_warns(
    tmp_path: Path, capsys: pytest.CaptureFixture[str]
) -> None:
    from icpp import library_stamp

    # A folder where the stamp of the first library goes
    (tmp_path / "liba" / library_stamp.STAMP_NAME).mkdir(parents=True)
    (tmp_path / "libb").mkdir()
    libraries = [{"lib_name": "liba"}, {"lib_name": "libb"}]
    library_stamp.after_build(tmp_path, libraries, None)
    stamp_a = tmp_path / "liba" / library_stamp.STAMP_NAME
    assert f"WARNING: cannot write {stamp_a}" in capsys.readouterr().out
    # The next library is stamped all the same
    stamp_b = tmp_path / "libb" / library_stamp.STAMP_NAME
    assert stamp_b.read_text("utf-8") == icpp.__version__


class CanisterCompiled(BaseException):
    """Stops the build once it reused the libraries and compiles the canister.
    Not an Exception, which the build would catch and turn into an exit."""


@pytest.mark.parametrize(
    "module, builder, root, ar_ext",
    [
        ("build_wasm", "build_library", "build-library", "WASM_AR_EXT"),
        (
            "build_native",
            "build_library_native",
            "build-library-native",
            "NATIVE_AR_EXT",
        ),
    ],
)
@pytest.mark.parametrize(
    "stamps, lib_names, rebuilds",
    [
        ({"libhello": "current", "__ic_candid__": "current"}, None, False),
        ({}, None, True),
        ({"libhello": "6.0.0", "__ic_candid__": "current"}, None, True),
        ({"libhello": "current", "__ic_candid__": "current"}, ["__ic_candid__"], True),
        ({"libhello": "unreadable", "__ic_candid__": "current"}, None, True),
        ({"libhello": "", "__ic_candid__": "current"}, None, True),
    ],
)
def test__mine_no_lib_builds_the_libraries_unless_this_version_built_them(
    project: Path,
    monkeypatch: pytest.MonkeyPatch,
    capsys: pytest.CaptureFixture[str],
    module: str,
    builder: str,
    root: str,
    ar_ext: str,
    stamps: Dict[str, str],
    lib_names: Optional[list[str]],
    rebuilds: bool,
) -> None:
    toml_text = WASM_TABLE + '\n[[build-library]]\nlib_name = "libhello"\n'
    load_icpp_toml(monkeypatch, toml_text)
    for lib_name in lib_names or ["libhello", "__ic_candid__"]:
        (project / root / lib_name).mkdir(parents=True)
        archive = f"{lib_name}{getattr(config_default, ar_ext)}"
        (project / root / lib_name / archive).write_text("", encoding="utf-8")
    for lib_name, stamp in stamps.items():
        stamp_path = stamp_of(project, root, lib_name)
        stamp_path.parent.mkdir(parents=True, exist_ok=True)
        if stamp == "unreadable":
            stamp_path.write_bytes(b"\xff")
        else:
            version = icpp.__version__ if stamp == "current" else stamp
            stamp_path.write_text(version, "utf-8")
    (project / "src" / "my_canister.did").write_text("service : {}", "utf-8")

    def compiled(*_a: Any, **_k: Any) -> None:
        raise CanisterCompiled

    command = importlib.import_module(f"icpp.commands_{module}")
    monkeypatch.setattr(command, builder, built)
    # The build of the canister's own code stops at its first command
    monkeypatch.setattr(command, "run_in_parallel", compiled)
    monkeypatch.setattr(command, "run_shell_cmd", compiled)
    with pytest.raises(LibrariesBuilt if rebuilds else CanisterCompiled):
        inspect.unwrap(getattr(command, module))(to_compile="mine-no-lib")
    assert ("building the libraries" in capsys.readouterr().out) == rebuilds


@pytest.mark.parametrize("inherit", [None, False, True])
def test__a_native_library_inherits_the_wasm_compile_flags_when_asked(
    project: Path, monkeypatch: pytest.MonkeyPatch, inherit: Optional[bool]
) -> None:
    from icpp.commands_build_library_native import build_library_native

    (project / "src" / "hello.cpp").write_text("", encoding="utf-8")
    toml_text = WASM_TABLE + (
        '\n[[build-library]]\nlib_name = "libhello"\ncpp_paths = ["src/hello.cpp"]\n'
        'cpp_compile_flags = ["-DFROM_WASM"]\n'
        '\n[[build-library-native]]\nlib_name = "libhello"\n'
        'cpp_compile_flags = ["-DFROM_NATIVE"]\n'
    )
    if inherit is not None:
        toml_text += f"inherit_compile_flags = {str(inherit).lower()}\n"
    load_icpp_toml(monkeypatch, toml_text)
    commands = stub_compiles(monkeypatch, "commands_build_library_native")
    inspect.unwrap(build_library_native)(lib_name="libhello")
    assert len(commands) == 1
    command = commands[0]
    assert "-DFROM_NATIVE" in command
    if inherit:
        assert command.index("-DFROM_WASM") < command.index("-DFROM_NATIVE")
    else:
        assert "-DFROM_WASM" not in command


def test__inherit_compile_flags_must_be_a_bool(
    project: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    del project
    toml_text = WASM_TABLE + (
        '\n[[build-library-native]]\nlib_name = "libhello"\n'
        'inherit_compile_flags = "yes"\n'
    )
    with pytest.raises(SystemExit):
        load_icpp_toml(monkeypatch, toml_text)


@pytest.mark.parametrize("size", [None, 4096, 8192, 16384, 32768, 65536])
def test__a_stable_fs_chunk_size_is_read(
    project: Path, monkeypatch: pytest.MonkeyPatch, size: Optional[int]
) -> None:
    del project
    toml_text = WASM_TABLE
    if size is not None:
        toml_text += f"stable_fs_chunk_size = {size}\n"
    icpp_toml = load_icpp_toml(monkeypatch, toml_text)
    # 0: the canister keeps the size its file system has
    assert icpp_toml.build_wasm["stable_fs_chunk_size"] == (size or 0)


@pytest.mark.parametrize("value", ["0", "1000", "131072", '"65536"', "true", "65536.0"])
def test__a_bad_stable_fs_chunk_size_exits(
    project: Path,
    monkeypatch: pytest.MonkeyPatch,
    capsys: pytest.CaptureFixture[str],
    value: str,
) -> None:
    del project
    with pytest.raises(SystemExit):
        load_icpp_toml(monkeypatch, WASM_TABLE + f"stable_fs_chunk_size = {value}\n")
    assert "'stable_fs_chunk_size' in '[build-wasm]' must be one of" in (
        capsys.readouterr().out
    )
