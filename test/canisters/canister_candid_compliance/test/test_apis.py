"""Run the Candid specification's test suite on a deployed canister

First deploy the canister, then run:

$ pytest --network=[local/ic]

Every assert of every suite file must pass.
"""

# pylint: disable=missing-function-docstring, unused-import, wildcard-import, unused-wildcard-import

from pathlib import Path

import pytest
from icpp.smoketest import call_canister_api

# Path to the icp.yaml file
ICP_YAML_PATH = Path(__file__).parent / "../icp.yaml"

# Canister in the icp.yaml file we want to test
CANISTER_NAME = "my_canister"

# The suite files, with their number of asserts
SUITES = {
    "official/construct.test.did": 169,
    "official/overshoot.test.did": 10,
    "official/prim.test.did": 168,
    "official/reference.test.did": 50,
    "official/spacebomb.test.did": 17,
    "official/subtypes.test.did": 58,
}


def test__suites(network: str) -> None:
    response = call_canister_api(
        icp_yaml_path=ICP_YAML_PATH,
        canister_name=CANISTER_NAME,
        canister_method="suites",
        network=network,
    )
    for path in SUITES:
        assert f'"{path}"' in response


@pytest.mark.parametrize("path,total", SUITES.items())
def test__run_suite(network: str, path: str, total: int) -> None:
    response = call_canister_api(
        icp_yaml_path=ICP_YAML_PATH,
        canister_name=CANISTER_NAME,
        canister_method="run_suite",
        canister_argument=f'("{path}")',
        network=network,
    )
    assert f"passed = {total} : nat64" in response, response
    assert f"total = {total} : nat64" in response, response
    assert 'failures = ""' in response, response
