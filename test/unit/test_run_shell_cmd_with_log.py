"""run_shell_cmd_with_log retries a failed command when asked to.

Regression: CI hit transient network errors in `icpp install-rust`, such as a
`git clone` whose connection was reset, and failed on the first attempt.
"""

import subprocess
from pathlib import Path

import pytest

from icpp import run_shell_cmd
from icpp.run_shell_cmd import run_shell_cmd_with_log

# Fails the first time it runs in a directory, succeeds after that.
FAILS_ONCE = "test -f ran_once || { touch ran_once; exit 1; }"


@pytest.fixture(autouse=True)
def no_retry_pause(monkeypatch: pytest.MonkeyPatch) -> None:
    """Keeps the tests fast."""
    monkeypatch.setattr(run_shell_cmd, "RETRY_PAUSE_SECONDS", 0)


def test__a_retry_recovers_from_a_transient_failure(tmp_path: Path) -> None:
    """The failed attempt is logged, then the retry succeeds."""
    log = tmp_path / "cmd.log"
    run_shell_cmd_with_log(log, "w", FAILS_ONCE, cwd=tmp_path, retries=1)
    assert log.read_text(encoding="utf-8").count(f"$ {FAILS_ONCE}") == 2


def test__without_retries_the_first_failure_raises(tmp_path: Path) -> None:
    """The default stays fail-fast."""
    with pytest.raises(subprocess.CalledProcessError):
        run_shell_cmd_with_log(tmp_path / "cmd.log", "w", FAILS_ONCE, cwd=tmp_path)


def test__raises_once_the_retries_are_used_up(tmp_path: Path) -> None:
    """A command that keeps failing is run 1 + retries times, then raises."""
    log = tmp_path / "cmd.log"
    with pytest.raises(subprocess.CalledProcessError):
        run_shell_cmd_with_log(log, "w", "exit 1", cwd=tmp_path, retries=2)
    assert log.read_text(encoding="utf-8").count("$ exit 1") == 3
