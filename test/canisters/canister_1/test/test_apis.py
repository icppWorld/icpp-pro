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
def test__roundtrip_bool_true(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_bool_true",
        canister_argument="(true)",
        network=network,
    )
    expected_response = "(true)"
    assert response == expected_response


def test__roundtrip_bool_false(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_bool_false",
        canister_argument="(false)",
        network=network,
    )
    expected_response = "(false)"
    assert response == expected_response


def test__roundtrip_nat_101(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_nat_101",
        canister_argument="(101 : nat)",
        network=network,
    )
    expected_response = "(101 : nat)"
    assert response == expected_response


def test__roundtrip_nat_1001(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_nat_1001",
        canister_argument="(1001 : nat)",
        network=network,
    )
    expected_response = "(1_001 : nat)"
    assert response == expected_response


def test__roundtrip_int_101(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_int_101",
        canister_argument="(101 : int)",
        network=network,
    )
    expected_response = "(101 : int)"
    assert response == expected_response


def test__roundtrip_int_1001(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_int_1001",
        canister_argument="(1001 : int)",
        network=network,
    )
    expected_response = "(1_001 : int)"
    assert response == expected_response


def test__roundtrip_int_101_neg(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_int_101_neg",
        canister_argument="(-101 : int)",
        network=network,
    )
    expected_response = "(-101 : int)"
    assert response == expected_response


def test__roundtrip_int_1001_neg(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_int_1001_neg",
        canister_argument="(-1001 : int)",
        network=network,
    )
    expected_response = "(-1_001 : int)"
    assert response == expected_response


def test__roundtrip_nat8_101(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_nat8_101",
        canister_argument="(101 : nat8)",
        network=network,
    )
    expected_response = "(101 : nat8)"
    assert response == expected_response


def test__roundtrip_nat16_101(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_nat16_101",
        canister_argument="(101 : nat16)",
        network=network,
    )
    expected_response = "(101 : nat16)"
    assert response == expected_response


def test__roundtrip_nat16_1001(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_nat16_1001",
        canister_argument="(1001 : nat16)",
        network=network,
    )
    expected_response = "(1_001 : nat16)"
    assert response == expected_response


def test__roundtrip_nat32_101(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_nat32_101",
        canister_argument="(101 : nat32)",
        network=network,
    )
    expected_response = "(101 : nat32)"
    assert response == expected_response


def test__roundtrip_nat32_1_000_000_001(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_nat32_1_000_000_001",
        canister_argument="(1_000_000_001 : nat32)",
        network=network,
    )
    expected_response = "(1_000_000_001 : nat32)"
    assert response == expected_response


def test__roundtrip_nat64_101(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_nat64_101",
        canister_argument="(101 : nat64)",
        network=network,
    )
    expected_response = "(101 : nat64)"
    assert response == expected_response


def test__roundtrip_nat64_1_000_000_000_000_000_001(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_nat64_1_000_000_000_000_000_001",
        canister_argument="(1_000_000_000_000_000_001 : nat64)",
        network=network,
    )
    expected_response = "(1_000_000_000_000_000_001 : nat64)"
    assert response == expected_response


def test__roundtrip_int8_101(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_int8_101",
        canister_argument="(101 : int8)",
        network=network,
    )
    expected_response = "(101 : int8)"
    assert response == expected_response


def test__roundtrip_int8_101_neg(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_int8_101_neg",
        canister_argument="(-101 : int8)",
        network=network,
    )
    expected_response = "(-101 : int8)"
    assert response == expected_response


def test__roundtrip_int16_101(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_int16_101",
        canister_argument="(101 : int16)",
        network=network,
    )
    expected_response = "(101 : int16)"
    assert response == expected_response


def test__roundtrip_int16_1001(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_int16_1001",
        canister_argument="(1001 : int16)",
        network=network,
    )
    expected_response = "(1_001 : int16)"
    assert response == expected_response


def test__roundtrip_int16_101_neg(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_int16_101_neg",
        canister_argument="(-101 : int16)",
        network=network,
    )
    expected_response = "(-101 : int16)"
    assert response == expected_response


def test__roundtrip_int16_1001_neg(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_int16_1001_neg",
        canister_argument="(-1001 : int16)",
        network=network,
    )
    expected_response = "(-1_001 : int16)"
    assert response == expected_response


def test__roundtrip_int32_101(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_int32_101",
        canister_argument="(101 : int32)",
        network=network,
    )
    expected_response = "(101 : int32)"
    assert response == expected_response


def test__roundtrip_int32_1_000_000_001(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_int32_1_000_000_001",
        canister_argument="(1_000_000_001 : int32)",
        network=network,
    )
    expected_response = "(1_000_000_001 : int32)"
    assert response == expected_response


def test__roundtrip_int32_101_neg(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_int32_101_neg",
        canister_argument="(-101 : int32)",
        network=network,
    )
    expected_response = "(-101 : int32)"
    assert response == expected_response


def test__roundtrip_int32_1_000_000_001_neg(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_int32_1_000_000_001_neg",
        canister_argument="(-1_000_000_001 : int32)",
        network=network,
    )
    expected_response = "(-1_000_000_001 : int32)"
    assert response == expected_response


def test__roundtrip_int64_101(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_int64_101",
        canister_argument="(101 : int64)",
        network=network,
    )
    expected_response = "(101 : int64)"
    assert response == expected_response


def test__roundtrip_int64_1_000_000_000_000_000_001(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_int64_1_000_000_000_000_000_001",
        canister_argument="(1_000_000_000_000_000_001 : int64)",
        network=network,
    )
    expected_response = "(1_000_000_000_000_000_001 : int64)"
    assert response == expected_response


def test__roundtrip_int64_101_neg(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_int64_101_neg",
        canister_argument="(-101 : int64)",
        network=network,
    )
    expected_response = "(-101 : int64)"
    assert response == expected_response


def test__roundtrip_int64_1_000_000_000_000_000_001_neg(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_int64_1_000_000_000_000_000_001_neg",
        canister_argument="(-1_000_000_000_000_000_001 : int64)",
        network=network,
    )
    expected_response = "(-1_000_000_000_000_000_001 : int64)"
    assert response == expected_response


def test__canister_roundtrip_text(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_text",
        canister_argument='("Hello C++ Canister")',
        network=network,
    )
    expected_response = '("Hello C++ Canister")'
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


def test__roundtrip_reserved(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_reserved",
        canister_argument="(null : reserved)",
        network=network,
    )
    expected_response = "(null : reserved)"
    assert response == expected_response


def test__roundtrip_float32(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_float32",
        canister_argument="(1001.1001 : float32)",
        network=network,
    )
    expected_response = "(1001.1001 : float32)"
    assert response == expected_response


def test__roundtrip_float32_neg(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_float32_neg",
        canister_argument="(-1001.1001 : float32)",
        network=network,
    )
    expected_response = "(-1001.1001 : float32)"
    assert response == expected_response


def test__roundtrip_float64(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_float64",
        canister_argument="(1001.1001 : float64)",
        network=network,
    )
    expected_response = "(1001.1001 : float64)"
    assert response == expected_response


def test__roundtrip_float64_neg(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_float64_neg",
        canister_argument="(-1001.1001 : float64)",
        network=network,
    )
    expected_response = "(-1001.1001 : float64)"
    assert response == expected_response


def test__roundtrip_record(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_record",
        canister_argument='(record {"name" = "C++ Developer"; "secret float64" = 0.01 : float64; "secret int" = 11 : int;})',
        network=network,
    )
    expected_response = '(record { secret float64 = 0.01 : float64; greeting = "Hello C++ Developer!"; secret int = 11 : int; message = "Your secret numbers are:";})'
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


def test__canister_sends_char_as_text(network: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="canister_sends_char_as_text",
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
