"""pytest fixtures provided by icpp
   https://docs.pytest.org/en/latest/fixture.html
"""

from typing import Any, Generator
import pytest


from icpp.smoketest import network_status, get_identity, set_identity, get_principal


def pytest_addoption(parser: Any) -> None:
    """Adds options: `pytest --network=[local/ic] `"""
    parser.addoption(
        "--network",
        action="store",
        default="local",
        help="The network to use: local or ic",
    )


@pytest.fixture(scope="session", autouse=True)
def network(request: Any) -> Any:
    """A fixture that verifies the network is up & returns the name."""
    network_ = request.config.getoption("--network")
    network_status(network_)
    return network_


def handle_identity(identity: str) -> Generator[dict[str, str], None, None]:
    """A fixture that sets the dfx identity."""
    identity_before_test = get_identity()
    set_identity(identity)
    user = {"identity": get_identity(), "principal": get_principal()}
    yield user
    set_identity(identity_before_test)


@pytest.fixture(scope="function")
def identity_anonymous() -> Generator[dict[str, str], None, None]:
    """A fixture that sets the dfx identity to anonymous."""
    yield from handle_identity("anonymous")


@pytest.fixture(scope="function")
def identity_default() -> Generator[dict[str, str], None, None]:
    """A fixture that sets the dfx identity to default."""
    yield from handle_identity("default")
