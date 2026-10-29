"""The smoketest helpers quote what they put in a shell command.

Regression: an argument with a single quote, such as `("it's")`, broke out of
the quotes around it, and the shell split it into other words.
"""

# pylint: disable=missing-function-docstring, redefined-outer-name, protected-access

import shlex
from pathlib import Path
from typing import List

import pytest

from icpp import smoketest


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
    assert words[3:6] == ["my_canister", "greet", '("it\'s; rm -rf ~")']
    assert words[words.index("--project-root-override") + 1].endswith("my project")


def test__is_query_in_did() -> None:
    did = 'service : { a : () -> (nat) query; "b" : (record { x: nat; }) -> (); }'
    assert smoketest._is_query_in_did(did, "a") is True
    assert smoketest._is_query_in_did(did, "b") is False
    assert smoketest._is_query_in_did(did, "c") is None
    # A name with no `->` after it: no later declaration has one either
    assert smoketest._is_query_in_did("type a = nat;", "a") is None
