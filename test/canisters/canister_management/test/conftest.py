"""The pytest fixtures
https://docs.pytest.org/en/latest/fixture.html
"""

# pylint: disable=missing-function-docstring, unused-import, wildcard-import, unused-wildcard-import, line-too-long, unused-argument
from pathlib import Path

import pytest
from icpp.conftest_base import *  # noqa: F403  # pytest fixtures provided by icpp-pro
from icpp.run_shell_cmd import run_shell_cmd


@pytest.fixture(scope="session", autouse=True)
def cycles_for_children(network: str, identity: str) -> None:
    """The tests create children with create_canister, each costing the
    creation fee plus the child's 100B cycles: top up canister_management once"""
    if network == "local":
        run_shell_cmd(
            f"icp canister top-up --amount 20t canister_management "
            f"--environment local --identity {identity}",
            cwd=Path(__file__).parent.parent,
        )
