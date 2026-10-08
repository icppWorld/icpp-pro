"""run_in_parallel must surface a failure, so a broken compile fails the build.

Regression: the parallel compile steps used a bare executor.map, which drops
a worker's exception unless its result is consumed. A C++ file that failed to
compile left `icpp build-wasm` exiting 0 with a wasm that lacked that file.
"""

import subprocess
import threading

import pytest

from icpp.run_shell_cmd import run_in_parallel


def test__raises_when_one_call_fails() -> None:
    """A single failing item fails the whole run, with its own error."""

    def compile_file(file: str) -> None:
        if file == "broken.cpp":
            raise subprocess.CalledProcessError(returncode=1, cmd=f"clang {file}")

    with pytest.raises(subprocess.CalledProcessError) as excinfo:
        run_in_parallel(compile_file, ["a.cpp", "broken.cpp", "b.cpp"])
    assert excinfo.value.cmd == "clang broken.cpp"


def test__runs_every_item_even_when_one_fails() -> None:
    """The other items still run; none are cut off by the failure."""
    done: list[str] = []
    lock = threading.Lock()

    def compile_file(file: str) -> None:
        if file == "broken.cpp":
            raise subprocess.CalledProcessError(returncode=1, cmd=file)
        with lock:
            done.append(file)

    with pytest.raises(subprocess.CalledProcessError):
        run_in_parallel(compile_file, ["a.cpp", "broken.cpp", "b.cpp"])
    assert sorted(done) == ["a.cpp", "b.cpp"]
