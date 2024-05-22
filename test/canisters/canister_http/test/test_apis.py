"""Test canister APIs

   First deploy the canister, then run:

   $ pytest --network=[local/ic]

"""

# pylint: disable=missing-function-docstring, unused-import, wildcard-import, unused-wildcard-import

from pathlib import Path
import json
import pytest
import requests
from icpp.smoketest import (
    call_canister_api,
    dict_to_candid_text,
    get_canister_url,
)

# Path to the dfx.json file
DFX_JSON_PATH = Path(__file__).parent / "../dfx.json"

# Canister in the dfx.json file we want to test
CANISTER_NAME = "my_canister"

#
# Note: network is specified on the pytest command:
#       pytest --network=[local/ic] ....
#

# -----------------------------------------------------------------------------------
# HTTPS INCALLS

# -----------------------------------------------------------------------------------
# HTTPS INCALL: GET /counter


def test__http_request_get_counter(network: str, principal: str) -> None:
    url = get_canister_url(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        network=network,
        url_path="counter",
    )
    response = requests.get(url)
    assert response.status_code == 200
    assert response.json() == {"counter": 0}


def test__http_request_post_counter(network: str, principal: str) -> None:
    # Increment the counter, using a POST request
    url = get_canister_url(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        network=network,
        url_path="counter",
    )
    response = requests.post(url)
    assert response.status_code == 200
    assert response.json() == {"counter": 1}

    # verify, by using a GET request, that Orthogonal Persistence worked
    url = get_canister_url(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        network=network,
        url_path="counter",
    )
    response = requests.get(url)
    assert response.status_code == 200
    assert response.json() == {"counter": 1}


def test__http_request_get_404_not_found(network: str, principal: str) -> None:
    url = get_canister_url(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        network=network,
        url_path="a-non-existing-path",
    )
    response = requests.get(url)
    assert response.status_code == 404
