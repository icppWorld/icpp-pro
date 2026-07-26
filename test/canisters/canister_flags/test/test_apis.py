"""Test canister APIs

First deploy the canister, then run:

$ pytest --network=[local/ic] test_apis.py

"""

# pylint: disable=missing-function-docstring, unused-import, wildcard-import, unused-wildcard-import, line-too-long

from pathlib import Path
from typing import Dict
import pytest
from icpp.smoketest import call_canister_api, dict_to_candid_text

# Path to the icp.yaml file
ICP_YAML_PATH = Path(__file__).parent / "../icp.yaml"

# Canister in the icp.yaml file we want to test
CANISTER_NAME = "my_canister"


def test__greet_json(network: str, principal: str) -> None:
    d = {"name": "AJB"}
    text_in = dict_to_candid_text(d)
    response = call_canister_api(
        icp_yaml_path=ICP_YAML_PATH,
        canister_name=CANISTER_NAME,
        canister_method="greet_json",
        canister_argument=text_in,
        network=network,
    )
    expected_response = '("{\\"greet\\":\\"Hello AJB!\\"}")'
    assert response == expected_response
