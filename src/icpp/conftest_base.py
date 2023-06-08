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
    pro.exit_if_not_pro("smoketesting with pytest")


###########################################################
# Some fixtures that always run, prior to the actual tests
# They verify everything is OK with the network & identity


@pytest.fixture(scope="session", autouse=True)
def network(_request: Any) -> Any:
    """A fixture that verifies the network is up & returns the name."""
    pro.exit_if_not_pro("smoketesting with pytest")


@pytest.fixture(scope="session", autouse=True)
def identity() -> Any:
    """A fixture that returns the name of the used identity."""
    pro.exit_if_not_pro("smoketesting with pytest")
    identity_ = get_identity()
    if identity_.startswith("ERROR"):
        raise Exception(identity_)
    return identity_


@pytest.fixture(scope="session", autouse=True)
def principal() -> Any:
    """A fixture that returns the principal of the used identity."""
    pro.exit_if_not_pro("smoketesting with pytest")
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


def handle_identity(identity_to_set: str) -> Generator[dict[str, str], None, None]:
    """A fixture that sets the dfx identity."""
    pro.exit_if_not_pro("smoketesting with pytest")
    yield {}


@pytest.fixture(scope="function")
def identity_anonymous() -> Generator[dict[str, str], None, None]:
    """A fixture that sets the dfx identity to anonymous."""
    pro.exit_if_not_pro("smoketesting with pytest")
    yield {}


@pytest.fixture(scope="function")
def identity_default() -> Generator[dict[str, str], None, None]:
    """A fixture that sets the dfx identity to default."""
    pro.exit_if_not_pro("smoketesting with pytest")
    yield {}
