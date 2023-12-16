"""Test canister APIs

   First deploy the canister, then run:

   $ pytest --network=[local/ic]

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

# ----------------------------------------------------------------------------------
# Run all roundtrip tests


# ------------------------------------------------------------------------
# Record with Vec Record field (headers of http_request)
def test__http_request_1(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="http_request",
        canister_argument="4449444c036c01c6a4a19806016d026c02f1fee18d0371cbe4fdc70471010003034831560348314e034832560348324e034833560348334e",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c096c02f1fee18d0301cbe4fdc704016d716c006c02f1fee18d0371cbe4fdc704716d036c02f1fee18d0371cbe4fdc704716c02f1fee18d0371cbe4fdc704716c02f1fee18d0371cbe4fdc704716c01c6a4a1980604010803034831560348314e034832560348324e034833560348334e"
    assert response == expected_response


def test__http_request_2(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="http_request",
        canister_argument="4449444c096c02f1fee18d0301cbe4fdc704016d716c006c02f1fee18d0371cbe4fdc704716d036c02f1fee18d0371cbe4fdc704716c02f1fee18d0371cbe4fdc704716c02f1fee18d0371cbe4fdc704716c01c6a4a1980604010803034831560348314e034832560348324e034833560348334e",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c096c02f1fee18d0301cbe4fdc704016d716c006c02f1fee18d0371cbe4fdc704716d036c02f1fee18d0371cbe4fdc704716c02f1fee18d0371cbe4fdc704716c02f1fee18d0371cbe4fdc704716c01c6a4a1980604010803034831560348314e034832560348324e034833560348334e"
    assert response == expected_response
