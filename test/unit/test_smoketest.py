"""The smoketest helpers quote what they put in a shell command.

Regression: an argument with a single quote, such as `("it's")`, broke out of
the quotes around it, and the shell split it into other words.
"""

# pylint: disable=missing-function-docstring, redefined-outer-name, protected-access

import shlex
import subprocess
from pathlib import Path
from typing import List, Optional

import pytest

from icpp import smoketest
from icpp.run_shell_cmd import run_shell_cmd


@pytest.fixture
def commands(monkeypatch: pytest.MonkeyPatch, tmp_path: Path) -> List[str]:
    """Records each shell command instead of running it."""
    recorded: List[str] = []

    def fake_run_shell_cmd(cmd: str, **_kwargs: object) -> str:
        recorded.append(cmd)
        return "()"

    monkeypatch.setattr(smoketest, "run_shell_cmd", fake_run_shell_cmd)
    monkeypatch.setattr(
        smoketest,
        "_verify_canister_name",
        lambda *_args: tmp_path / "my project" / "icp.yaml",
    )
    return recorded


def test__call_canister_api_quotes_its_values(commands: List[str]) -> None:
    smoketest.call_canister_api(
        canister_name="my_canister",
        canister_method="greet",
        canister_argument='("it\'s; rm -rf ~")',
        query=False,
        identity="icpp-pro-testing",
    )
    words = shlex.split(commands[0])
    assert words[:2] == ["exec", "icp"]
    assert words[4:7] == ["my_canister", "greet", '("it\'s; rm -rf ~")']
    assert words[words.index("--project-root-override") + 1].endswith("my project")


def test__call_canister_api_passes_a_large_argument_through_a_file(
    monkeypatch: pytest.MonkeyPatch, tmp_path: Path
) -> None:
    """Linux caps one argument at 128 KiB, and the command is one argument of
    /bin/sh: a large argument goes to icp in a file, removed afterwards."""
    argument = '("' + "x" * 200 * 1024 + '")'
    seen: List[str] = []
    args_files: List[Path] = []

    def fake_run_shell_cmd(cmd: str, **_kwargs: object) -> str:
        seen.append(cmd)
        words = shlex.split(cmd)
        args_file = Path(words[words.index("--args-file") + 1])
        args_files.append(args_file)
        assert args_file.read_text(encoding="utf-8") == argument
        return "()"

    monkeypatch.setattr(smoketest, "run_shell_cmd", fake_run_shell_cmd)
    monkeypatch.setattr(
        smoketest, "_verify_canister_name", lambda *_args: tmp_path / "icp.yaml"
    )
    smoketest.call_canister_api(
        canister_name="my_canister",
        canister_method="greet",
        canister_argument=argument,
        query=False,
        identity="icpp-pro-testing",
    )
    assert len(seen[0]) < 1024
    assert not args_files[0].exists()


def _timeouts_of_calls(
    monkeypatch: pytest.MonkeyPatch,
    tmp_path: Path,
    argument: str,
    timeout_seconds: Optional[int] = None,
) -> List[object]:
    """Calls call_canister_api, recording the timeout each shell command got."""
    timeouts: List[object] = []

    def fake_run_shell_cmd(_cmd: str, **kwargs: object) -> str:
        timeouts.append(kwargs.get("timeout_seconds"))
        return "()"

    monkeypatch.setattr(smoketest, "run_shell_cmd", fake_run_shell_cmd)
    monkeypatch.setattr(
        smoketest, "_verify_canister_name", lambda *_args: tmp_path / "icp.yaml"
    )
    smoketest.call_canister_api(
        canister_name="my_canister",
        canister_method="greet",
        canister_argument=argument,
        query=False,
        identity="icpp-pro-testing",
        timeout_seconds=timeout_seconds,
    )
    return timeouts


def test__call_canister_api_gives_a_large_argument_a_longer_timeout(
    monkeypatch: pytest.MonkeyPatch, tmp_path: Path
) -> None:
    """icp can take longer than 30 seconds to send an argument near 2 MB."""
    large = '("' + "x" * 200 * 1024 + '")'
    # None: run_shell_cmd's own default of 30 seconds
    assert _timeouts_of_calls(monkeypatch, tmp_path, '("hi")') == [None]
    assert _timeouts_of_calls(monkeypatch, tmp_path, large) == [
        smoketest._ARGS_FILE_TIMEOUT_SECONDS
    ]
    assert _timeouts_of_calls(monkeypatch, tmp_path, large, 7) == [7]


def test__call_canister_api_reports_a_timeout_as_a_failed_call(
    monkeypatch: pytest.MonkeyPatch, tmp_path: Path
) -> None:
    def fake_run_shell_cmd(cmd: str, **kwargs: object) -> str:
        assert kwargs.get("raise_on_timeout") is True
        raise subprocess.TimeoutExpired(cmd, 30)

    monkeypatch.setattr(smoketest, "run_shell_cmd", fake_run_shell_cmd)
    monkeypatch.setattr(
        smoketest, "_verify_canister_name", lambda *_args: tmp_path / "icp.yaml"
    )
    response = smoketest.call_canister_api(
        canister_name="my_canister",
        canister_method="greet",
        query=False,
        identity="icpp-pro-testing",
    )
    assert response == (
        "Failed call to api 'greet' of canister 'my_canister': "
        "timed out after 30 seconds, with output: "
    )


def test__call_canister_api_does_not_cache_an_interface_lookup_that_timed_out(
    monkeypatch: pytest.MonkeyPatch, tmp_path: Path
) -> None:
    lookups: List[str] = []

    def fake_run_shell_cmd(cmd: str, **_kwargs: object) -> str:
        if "candid:service" in cmd:
            lookups.append(cmd)
            raise subprocess.TimeoutExpired(cmd, 30)
        return "()"

    monkeypatch.setattr(smoketest, "run_shell_cmd", fake_run_shell_cmd)
    monkeypatch.setattr(
        smoketest, "_verify_canister_name", lambda *_args: tmp_path / "icp.yaml"
    )
    monkeypatch.setattr(smoketest, "_canister_wasm_path", lambda *_args: None)
    monkeypatch.setattr(smoketest, "_CANDID_CACHE", {})
    for _ in range(2):
        smoketest.call_canister_api(
            canister_name="my_canister",
            canister_method="greet",
            identity="icpp-pro-testing",
        )
    assert len(lookups) == 2


def test__run_shell_cmd_raises_on_a_timeout_when_asked(
    monkeypatch: pytest.MonkeyPatch,
) -> None:
    def fake_run(cmd: str, **_kwargs: object) -> object:
        raise subprocess.TimeoutExpired(cmd, 1, output="partial")

    monkeypatch.setattr(subprocess, "run", fake_run)
    assert run_shell_cmd("icp x", capture_output=True, timeout_seconds=1).startswith(
        "ERROR: Command 'icp x' timed out after 1 seconds, with stdout: "
    )
    with pytest.raises(subprocess.TimeoutExpired):
        run_shell_cmd(
            "icp x", capture_output=True, timeout_seconds=1, raise_on_timeout=True
        )


def test__is_query_in_did() -> None:
    did = 'service : { a : () -> (nat) query; "b" : (record { x: nat; }) -> (); }'
    assert smoketest._is_query_in_did(did, "a") is True
    assert smoketest._is_query_in_did(did, "b") is False
    assert smoketest._is_query_in_did(did, "c") is None
    # A name with no `->` after it: no later declaration has one either
    assert smoketest._is_query_in_did("type a = nat;", "a") is None
