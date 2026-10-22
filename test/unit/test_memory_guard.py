"""The memory guard of `icpp run-native` kills a run over its memory limit,
fails closed when it cannot measure, and passes the exit code through."""

import os
import subprocess
import sys
from pathlib import Path

import pytest

from icpp.memory_guard import guard

HOG = [sys.executable, "-c", "import time; x = bytearray(300 * 2**20); time.sleep(30)"]
SMALL = [sys.executable, "-c", "x = bytearray(40 * 2**20)"]


def test__kills_a_run_over_the_limit(
    capsys: pytest.CaptureFixture[str], monkeypatch: pytest.MonkeyPatch
) -> None:
    """Over the limit: killed, an ERROR and a GitHub annotation, exit 1."""
    monkeypatch.setenv("GITHUB_ACTIONS", "true")
    assert guard(HOG, 100 * 2**20, interval=0.1) == 1
    captured = capsys.readouterr()
    assert "over the 100.0 MiB limit of the memory guard: killed" in captured.err
    assert "::error title=memory guard::" in captured.out


def test__passes_the_exit_code_through() -> None:
    """Under the limit, the command's own exit code is returned."""
    assert guard([sys.executable, "-c", "raise SystemExit(3)"], 2**30) == 3
    assert guard([sys.executable, "-c", "pass"], 2**30) == 0


def test__fails_closed_without_ps(
    capsys: pytest.CaptureFixture[str], monkeypatch: pytest.MonkeyPatch
) -> None:
    """When ps is missing, the command is killed and the run fails."""
    monkeypatch.setenv("PATH", "/nonexistent")
    command = [sys.executable, "-c", "import time; time.sleep(30)"]
    assert guard(command, 2**30, interval=0.1) == 1
    assert "cannot run ps to measure" in capsys.readouterr().err


def test__reports_the_peak_of_a_short_run(capsys: pytest.CaptureFixture[str]) -> None:
    """A run that ends before the first check still reports its peak."""
    assert guard(SMALL, 2**30) == 0
    line = [l for l in capsys.readouterr().out.splitlines() if "[memory guard]" in l]
    assert len(line) == 1
    assert float(line[0].split("peak ")[1].split(" MiB")[0]) >= 40


def test__run_native_needs_a_build(tmp_path: Path) -> None:
    """icpp run-native fails clearly when there is no build-native/mockic.exe."""
    result = subprocess.run(
        [str(Path(sys.executable).parent / "icpp"), "run-native"],
        cwd=tmp_path,
        capture_output=True,
        text=True,
        check=False,
        env={**os.environ},
    )
    assert result.returncode == 1
    assert "Run 'icpp build-native' first" in result.stdout
