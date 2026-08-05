"""pytest fixtures provided by icpp-pro
https://docs.pytest.org/en/latest/fixture.html

Tests run as an identity you name explicitly, either with
`pytest --identity <name>` or by exporting ICPP_PRO_TEST_IDENTITY. That name is
passed to every icp command as `--identity <name>`.

The machine-wide active identity (`icp identity default`) is never read and
never written. It is shared by every process on the machine, so scoping a
test's identity through it would both clobber the identity you use for mainnet
work and leave the test at the mercy of anything else that changes it mid-run.
"""

# A fixture that uses another fixture takes it as a parameter of the same name,
# which is exactly what pylint's redefined-outer-name flags.
# pylint: disable=redefined-outer-name

import os
from typing import Any, Generator, Dict
import pytest


from icpp.smoketest import (
    IDENTITY_ENV_VAR,
    get_principal,
    network_status,
    no_identity_msg,
    set_identity_override,
    set_session_identity,
)


def pytest_addoption(parser: Any) -> None:
    """Adds options: `pytest --network=[local/ic] --identity=<name>`"""
    parser.addoption(
        "--network",
        action="store",
        default="local",
        help=(
            "The icp.yaml environment to use, eg. local or ic. "
            "It is passed to icp as `--environment`."
        ),
    )
    parser.addoption(
        "--identity",
        action="store",
        default=None,
        help=(
            "The icp identity to run the tests as. It is passed to icp as "
            f"`--identity`. Falls back to ${IDENTITY_ENV_VAR}. Required: "
            "icpp-pro never uses the machine-wide active identity."
        ),
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
def identity(request: Any) -> Any:
    """A fixture that pins & returns the identity the tests run as.

    Resolved once, from `--identity` or ${ICPP_PRO_TEST_IDENTITY}, and then
    passed explicitly to every icp command. Nothing re-reads it, so a change
    to the machine's active identity mid-run cannot affect the tests.
    """
    identity_ = request.config.getoption("--identity") or os.environ.get(
        IDENTITY_ENV_VAR
    )
    if not identity_:
        pytest.exit(no_identity_msg())

    set_session_identity(identity_)
    return identity_


@pytest.fixture(scope="session", autouse=True)
def principal(identity: str) -> Any:
    """A fixture that returns the principal of the identity the tests run as.

    Asking for it up front turns a misspelled or password-protected identity
    into one clear error at session start, instead of an obscure failure in
    whichever test happens to run first.
    """
    principal_ = get_principal(identity=identity)
    if principal_.startswith("ERROR"):
        # A password-protected identity makes icp prompt. Depending on whether
        # a terminal is attached, that either says so or simply blocks until
        # the command times out - so treat both as the same diagnosis.
        lowered = principal_.lower()
        if any(s in lowered for s in ("password", "passphrase", "timed out")):
            msg = (
                f"Identity '{identity}' looks password protected. icpp-pro "
                f"exports the key to sign locally, so use an identity created "
                f"with '--storage plaintext'!\n\n{principal_}"
            )
            raise RuntimeError(msg)
        raise RuntimeError(principal_)
    return principal_


####################################################################
# Fixtures to run a function with the anonymous or session identity


@pytest.fixture(scope="session")
def identity_default(identity: str, principal: str) -> Dict[str, str]:
    """A fixture that returns the identity the tests run as.

    Despite the name, this is not an identity literally called `default` - it
    is the one named by `pytest --identity` or ${ICPP_PRO_TEST_IDENTITY}, which
    is also the identity the canister was deployed with. The name is kept for
    backwards compatibility.
    """
    return {"identity": identity, "principal": principal}


@pytest.fixture(scope="function")
def identity_anonymous() -> Generator[Dict[str, str], None, None]:
    """A fixture that runs a test's canister calls as the anonymous identity.

    The override lives inside this pytest process, so it is invisible to every
    other process on the machine and there is nothing to restore if the run is
    killed.
    """
    set_identity_override("anonymous")
    try:
        yield {"identity": "anonymous", "principal": "2vxsx-fae"}
    finally:
        set_identity_override(None)
