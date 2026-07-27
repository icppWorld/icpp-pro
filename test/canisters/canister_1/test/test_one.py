"""Test canister APIs - just one test

First deploy the canister, then run:

$ pytest --network=[local/ic] test/test_one.py

"""

# pylint: disable=missing-function-docstring, unused-import, wildcard-import, unused-wildcard-import

from pathlib import Path
import json
import pytest
from icpp.smoketest import call_canister_api, dict_to_candid_text

# Path to the icp.yaml file
ICP_YAML_PATH = Path(__file__).parent / "../icp.yaml"

# Canister in the icp.yaml file we want to test
CANISTER_NAME = "my_canister"

#
# Note: network is specified on the pytest command:
#       pytest --network=[local/ic] ....
#


def test__roundtrip_bool_true(network: str, principal: str) -> None:
    response = call_canister_api(
        icp_yaml_path=ICP_YAML_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_bool_true",
        canister_argument="(true)",
        network=network,
    )
    expected_response = "(true)"
    assert response == expected_response
