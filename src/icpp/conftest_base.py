"""pytest fixtures provided by icpp
   https://docs.pytest.org/en/latest/fixture.html
"""
# pylint: disable=unused-import, unused-argument
from typing import Any, Generator
import pytest

from icpp.smoketest import (
    network_status,
    get_identity,
    set_identity,
    get_principal,
)
from icpp import pro


def pytest_addoption(parser: Any) -> None:
    """Adds options: `pytest --network=[local/ic] `"""
    pro.exit_if_not_pro()


@pytest.fixture(scope="session", autouse=True)
def network(_request: Any) -> Any:
    """A fixture that verifies the network is up & returns the name."""
    pro.exit_if_not_pro()


def handle_identity(identity: str) -> Generator[dict[str, str], None, None]:
    """A fixture that sets the dfx identity."""
    pro.exit_if_not_pro()
    yield {}


@pytest.fixture(scope="function")
def identity_anonymous() -> Generator[dict[str, str], None, None]:
    """A fixture that sets the dfx identity to anonymous."""
    pro.exit_if_not_pro()
    yield {}


@pytest.fixture(scope="function")
def identity_default() -> Generator[dict[str, str], None, None]:
    """A fixture that sets the dfx identity to default."""
    pro.exit_if_not_pro()
    yield {}
