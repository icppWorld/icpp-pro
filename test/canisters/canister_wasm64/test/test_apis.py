"""Test canister APIs

   First deploy the canister, then run:

   $ pytest --network=[local/ic] test_apis.py

"""
# pylint: disable=missing-function-docstring, unused-import, wildcard-import, unused-wildcard-import, line-too-long

from pathlib import Path
from typing import Dict
import pytest
from icpp.smoketest import call_canister_api, dict_to_candid_text

# Path to the dfx.json file
DFX_JSON_PATH = Path(__file__).parent / "../dfx.json"

# Canister in the dfx.json file we want to test
CANISTER_NAME = "greet"


def test__query_greet(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="query_greet",
        canister_argument="()",
        network=network,
    )
    expected_response = '("hello!")'
    assert response == expected_response


def test__query_greet32(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="query_greet32",
        canister_argument="()",
        network=network,
    )
    expected_response = '("hello 32!")'
    assert response == expected_response


def test__query_greet64(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="query_greet64",
        canister_argument="()",
        network=network,
    )
    expected_response = '("hello 64!")'
    assert response == expected_response


def test__update_greet(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="update_greet",
        canister_argument="()",
        network=network,
    )
    expected_response = '("hello!")'
    assert response == expected_response


def test__update_greet32(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="update_greet32",
        canister_argument="()",
        network=network,
    )
    expected_response = '("hello 32!")'
    assert response == expected_response


def test__update_greet64(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="update_greet64",
        canister_argument="()",
        network=network,
    )
    expected_response = '("hello 64!")'
    assert response == expected_response
