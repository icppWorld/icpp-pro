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
    get_canister_url_with_headers,
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
    url, headers = get_canister_url_with_headers(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        network=network,
        url_path="counter",
    )
    response = requests.get(url, headers=headers)
    assert response.status_code == 200
    assert "counter" in response.json().keys()


def test__http_request_post_counter(network: str, principal: str) -> None:
    url, headers = get_canister_url_with_headers(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        network=network,
        url_path="counter",
    )
    response = requests.get(url, headers=headers)
    assert response.status_code == 200
    counter_before = response.json()["counter"]
    print(f"counter_before = {counter_before}")

    # Increment the counter, using a POST request
    url, headers = get_canister_url_with_headers(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        network=network,
        url_path="counter",
    )
    response = requests.post(url, headers=headers)
    assert response.status_code == 200
    assert "counter" in response.json().keys()

    # verify, by using a GET request, that Orthogonal Persistence worked
    url, headers = get_canister_url_with_headers(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        network=network,
        url_path="counter",
    )
    response = requests.get(url, headers=headers)
    assert response.status_code == 200
    counter_after = response.json()["counter"]
    print(f"counter_after = {counter_after}")
    assert counter_after - counter_before == 1


def test__http_request_get_404_not_found(network: str, principal: str) -> None:
    url, headers = get_canister_url_with_headers(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        network=network,
        url_path="unknown-path",
    )
    response = requests.get(url, headers=headers)
    print(f"response = {response}")
    assert response.status_code == 404
