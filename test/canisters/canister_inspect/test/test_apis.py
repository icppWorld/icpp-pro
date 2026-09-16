"""Test canister APIs

First deploy the canister, then run:

$ pytest --network=[local/ic]

The canister exports a canister_inspect_message hook that accepts only
ingress calls to methods whose name starts with "allowed_".
"""

# pylint: disable=missing-function-docstring, unused-import, wildcard-import, unused-wildcard-import

from pathlib import Path
from icpp.smoketest import call_canister_api

# Path to the icp.yaml file
ICP_YAML_PATH = Path(__file__).parent / "../icp.yaml"

# Canister in the icp.yaml file we want to test
CANISTER_NAME = "my_canister"


def test__allowed_greet(network: str) -> None:
    response = call_canister_api(
        icp_yaml_path=ICP_YAML_PATH,
        canister_name=CANISTER_NAME,
        canister_method="allowed_greet",
        network=network,
    )
    assert response == '("Hello!")'


def test__blocked_greet_is_refused(network: str) -> None:
    # The inspect hook never calls accept_message for this method name, so
    # the ingress message is rejected before execution. Assert the
    # rejection-specific marker (IC0406 = canister rejected the message),
    # not just any failed call.
    response = call_canister_api(
        icp_yaml_path=ICP_YAML_PATH,
        canister_name=CANISTER_NAME,
        canister_method="blocked_greet",
        network=network,
    )
    assert response.startswith("Failed call to api")
    assert "Canister rejected the message" in response
