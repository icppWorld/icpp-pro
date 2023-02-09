"""Test canister APIs

   First deploy the canister, then run:

   $ pytest --network=[local/ic] test__canister_1.py

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


# Unit tests for ic_api
def test__test_ic_api(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="test_ic_api",
        network=network,
    )
    expected_response = "(0 : int)"
    assert response == expected_response


# Unit & compliance tests for candid
def test__test_candid(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="test_candid",
        network=network,
    )
    expected_response = "(0 : int)"
    assert response == expected_response


# Unit tests for 3rd party vendor libraries
def test__test_vendors(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="test_vendors",
        network=network,
    )
    expected_response = "(0 : int)"
    assert response == expected_response


# Roundtrip tests
def test__roundtrip_bool_1(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_bool",
        canister_argument="(true)",
        network=network,
    )
    expected_response = "(true)"
    assert response == expected_response


def test__roundtrip_bool_2(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_bool",
        canister_argument="(false)",
        network=network,
    )
    expected_response = "(false)"
    assert response == expected_response


# Older tests for sending only
def test__canister_sends_int_as_int(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="canister_sends_int_as_int",
        network=network,
    )
    expected_response = "(101 : int)"
    assert response == expected_response


def test__canister_sends_double_as_float64(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="canister_sends_double_as_float64",
        network=network,
    )
    expected_response = "(1001.1001 : float64)"
    assert response == expected_response


def test__canister_sends_char_as_text(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="canister_sends_char_as_text",
        network=network,
    )
    expected_response = '("Hello!!!")'
    assert response == expected_response


def test__canister_sends_string_as_text(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="canister_sends_string_as_text",
        network=network,
    )
    expected_response = '("Hello!!!")'
    assert response == expected_response


def test__canister_sends_json_as_text(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="canister_sends_json_as_text",
        network=network,
    )
    expected_response = '("{\\"happy\\":true,\\"pi\\":3.141}")'
    assert response == expected_response


def test__roundtrip_text_to_json_to_text(network: str) -> None:
    d = {"project": "?", "C++": 0}
    text_in = dict_to_candid_text(d)
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_text_to_json_to_text",
        canister_argument=text_in,
        network=network,
    )
    expected_response = '("{\\"C++\\":20,\\"project\\":\\"icpp\\"}")'
    assert response == expected_response


def test__roundtrip_text_to_json_to_text__long_message(network: str) -> None:
    """Verify that api can handle very long messages"""
    d = {
        "project": "icpp",
        "C++": 20,
        "a_long_message": "abcdefghijklmnopqrstuvwxyz01abcdefghijklmnopqrstuvwxyz02abcdefghijklmnopqrstuvwxyz03abcdefghijklmnopqrstuvwxyz04abcdefghijklmnopqrstuvwxyz05abcdefghijklmnopqrstuvwxyz06abcdefghijklmnopqrstuvwxyz07abcdefghijklmnopqrstuvwxyz08abcdefghijklmnopqrstuvwxyz09abcdefghijklmnopqrstuvwxyz10abcdefghijklmnopqrstuvwxyz11abcdefghijklmnopqrstuvwxyz12abcdefghijklmnopqrstuvwxyz13",  # pylint: disable=line-too-long
    }
    text_in = dict_to_candid_text(d)
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_text_to_json_to_text",
        canister_argument=text_in,
        network=network,
    )
    expected_response = '("{\\"C++\\":20,\\"a_long_message\\":\\"abcdefghijklmnopqrstuvwxyz01abcdefghijklmnopqrstuvwxyz02abcdefghijklmnopqrstuvwxyz03abcdefghijklmnopqrstuvwxyz04abcdefghijklmnopqrstuvwxyz05abcdefghijklmnopqrstuvwxyz06abcdefghijklmnopqrstuvwxyz07abcdefghijklmnopqrstuvwxyz08abcdefghijklmnopqrstuvwxyz09abcdefghijklmnopqrstuvwxyz10abcdefghijklmnopqrstuvwxyz11abcdefghijklmnopqrstuvwxyz12abcdefghijklmnopqrstuvwxyz13\\",\\"project\\":\\"icpp\\"}")'  # pylint: disable=line-too-long
    assert response == expected_response


def test__roundtrip_text_to_json_to_text__negative(network: str) -> None:
    """Verify that api traps when it receives a wrong message"""
    text_in = json.dumps('("")')  # escapes the "
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_text_to_json_to_text",
        canister_argument=text_in,
        network=network,
    )
    assert "trapped: unreachable" in response


def test__roundtrip_text_to_json_to_text__negative_2(network: str) -> None:
    """Verify that api traps when it receives a wrong message"""
    text_in = dict_to_candid_text(({"blabla": 1000}))
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_text_to_json_to_text",
        canister_argument=text_in,
        network=network,
    )
    assert "trapped explicitly" in response
