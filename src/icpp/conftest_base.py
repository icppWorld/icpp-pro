"""pytest fixtures provided by icpp-pro
   https://docs.pytest.org/en/latest/fixture.html
"""

from typing import Any, Generator, Dict
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


###########################################################
# Some fixtures that always run, prior to the actual tests
# They verify everything is OK with the network & identity


@pytest.fixture(scope="session", autouse=True)
def network(request: Any) -> Any:
    """A fixture that verifies the network is up & returns the name."""
    network_ = request.config.getoption("--network")
    network_status(network_)
    return network_


@pytest.fixture(scope="session", autouse=True)
def identity() -> Any:
    """A fixture that returns the name of the used identity."""
    identity_ = get_identity()
    if identity_.startswith("ERROR"):
        raise Exception(identity_)
    return identity_


@pytest.fixture(scope="session", autouse=True)
def principal() -> Any:
    """A fixture that returns the principal of the used identity."""
    principal_ = get_principal()
    if principal_.startswith("ERROR"):
        if "Please enter the passphrase for your identity" in principal_:
            msg = (
                f"Identity '{get_identity()}' uses a passphrase. "
                f"Use identity created with '--storage-mode plaintext'!"
            )
            raise Exception(msg)
        raise Exception(principal_)
    return principal_


####################################################################
# Fixtures to run a function with the anonymous or default identity


def handle_identity(identity_to_set: str) -> Generator[Dict[str, str], None, None]:
    """A fixture that sets the dfx identity."""
    identity_before_test = get_identity()
    set_identity(identity_to_set)
    user = {"identity": get_identity(), "principal": get_principal()}
    yield user
    set_identity(identity_before_test)


@pytest.fixture(scope="function")
def identity_anonymous() -> Generator[Dict[str, str], None, None]:
    """A fixture that sets the dfx identity to anonymous."""
    yield from handle_identity("anonymous")


@pytest.fixture(scope="function")
def identity_default() -> Generator[Dict[str, str], None, None]:
    """A fixture that sets the dfx identity to default."""
    yield from handle_identity("default")
