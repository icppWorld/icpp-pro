"""Test canister APIs - just one test

   First deploy the canister, then run:

   $ pytest --network=[local/ic] test/test_one.py

"""

# pylint: disable=missing-function-docstring, unused-import, wildcard-import, unused-wildcard-import

from pathlib import Path
import json
import pytest
from icpp.smoketest import call_canister_api, dict_to_candid_text

# Path to the dfx.json file
DFX_JSON_PATH = Path(__file__).parent / "../dfx.json"

# Canister in the dfx.json file we want to test
CANISTER_NAME = "my_canister"

#
# Note: network is specified on the pytest command:
#       pytest --network=[local/ic] ....
#


def test__roundtrip_bool_true(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_bool_true",
        canister_argument="(true)",
        network=network,
    )
    expected_response = "(true)"
    assert response == expected_response
