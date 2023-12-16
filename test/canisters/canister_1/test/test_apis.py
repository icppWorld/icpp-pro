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
# Run all unit tests for vendor libraries
def test__test_vendors(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="test_vendors",
        network=network,
    )
    expected_response = "(0 : int)"
    assert response == expected_response


# ----------------------------------------------------------------------------------
# Run all unit & compliance tests for candid
def test__test_candid(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="test_candid",
        network=network,
    )
    expected_response = "(0 : int)"
    assert response == expected_response


# ----------------------------------------------------------------------------------
# Run all unit tests for ic_api
def test__test_ic_api(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="test_ic_api",
        network=network,
    )
    expected_response = "(0 : int)"
    assert response == expected_response


# ----------------------------------------------------------------------------------
# Run all roundtrip tests
def test__roundtrip_deprecated_ic_api_constructor(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_deprecated_ic_api_constructor",
        canister_argument="()",
        network=network,
    )
    expected_response = ""
    assert response == expected_response


def test__roundtrip_no_arguments(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_no_arguments",
        canister_argument="()",
        network=network,
    )
    expected_response = ""
    assert response == expected_response


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


def test__roundtrip_bool_false(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_bool_false",
        canister_argument="(false)",
        network=network,
    )
    expected_response = "(false)"
    assert response == expected_response


def test__roundtrip_nat_101(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_nat_101",
        canister_argument="(101 : nat)",
        network=network,
    )
    expected_response = "(101 : nat)"
    assert response == expected_response


def test__roundtrip_nat_1001(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_nat_1001",
        canister_argument="(1001 : nat)",
        network=network,
    )
    expected_response = "(1_001 : nat)"
    assert response == expected_response


def test__roundtrip_int_101(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_int_101",
        canister_argument="(101 : int)",
        network=network,
    )
    expected_response = "(101 : int)"
    assert response == expected_response


def test__roundtrip_int_1001(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_int_1001",
        canister_argument="(1001 : int)",
        network=network,
    )
    expected_response = "(1_001 : int)"
    assert response == expected_response


def test__roundtrip_int_101_neg(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_int_101_neg",
        canister_argument="(-101 : int)",
        network=network,
    )
    expected_response = "(-101 : int)"
    assert response == expected_response


def test__roundtrip_int_1001_neg(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_int_1001_neg",
        canister_argument="(-1001 : int)",
        network=network,
    )
    expected_response = "(-1_001 : int)"
    assert response == expected_response


def test__roundtrip_nat8_101(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_nat8_101",
        canister_argument="(101 : nat8)",
        network=network,
    )
    expected_response = "(101 : nat8)"
    assert response == expected_response


def test__roundtrip_nat16_101(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_nat16_101",
        canister_argument="(101 : nat16)",
        network=network,
    )
    expected_response = "(101 : nat16)"
    assert response == expected_response


def test__roundtrip_nat16_1001(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_nat16_1001",
        canister_argument="(1001 : nat16)",
        network=network,
    )
    expected_response = "(1_001 : nat16)"
    assert response == expected_response


def test__roundtrip_nat32_101(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_nat32_101",
        canister_argument="(101 : nat32)",
        network=network,
    )
    expected_response = "(101 : nat32)"
    assert response == expected_response


def test__roundtrip_nat32_1_000_000_001(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_nat32_1_000_000_001",
        canister_argument="(1_000_000_001 : nat32)",
        network=network,
    )
    expected_response = "(1_000_000_001 : nat32)"
    assert response == expected_response


def test__roundtrip_nat64_101(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_nat64_101",
        canister_argument="(101 : nat64)",
        network=network,
    )
    expected_response = "(101 : nat64)"
    assert response == expected_response


def test__roundtrip_nat64_1_000_000_000_000_000_001(
    network: str, principal: str
) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_nat64_1_000_000_000_000_000_001",
        canister_argument="(1_000_000_000_000_000_001 : nat64)",
        network=network,
    )
    expected_response = "(1_000_000_000_000_000_001 : nat64)"
    assert response == expected_response


def test__roundtrip_int8_101(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_int8_101",
        canister_argument="(101 : int8)",
        network=network,
    )
    expected_response = "(101 : int8)"
    assert response == expected_response


def test__roundtrip_int8_101_neg(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_int8_101_neg",
        canister_argument="(-101 : int8)",
        network=network,
    )
    expected_response = "(-101 : int8)"
    assert response == expected_response


def test__roundtrip_int16_101(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_int16_101",
        canister_argument="(101 : int16)",
        network=network,
    )
    expected_response = "(101 : int16)"
    assert response == expected_response


def test__roundtrip_int16_1001(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_int16_1001",
        canister_argument="(1001 : int16)",
        network=network,
    )
    expected_response = "(1_001 : int16)"
    assert response == expected_response


def test__roundtrip_int16_101_neg(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_int16_101_neg",
        canister_argument="(-101 : int16)",
        network=network,
    )
    expected_response = "(-101 : int16)"
    assert response == expected_response


def test__roundtrip_int16_1001_neg(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_int16_1001_neg",
        canister_argument="(-1001 : int16)",
        network=network,
    )
    expected_response = "(-1_001 : int16)"
    assert response == expected_response


def test__roundtrip_int32_101(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_int32_101",
        canister_argument="(101 : int32)",
        network=network,
    )
    expected_response = "(101 : int32)"
    assert response == expected_response


def test__roundtrip_int32_1_000_000_001(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_int32_1_000_000_001",
        canister_argument="(1_000_000_001 : int32)",
        network=network,
    )
    expected_response = "(1_000_000_001 : int32)"
    assert response == expected_response


def test__roundtrip_int32_101_neg(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_int32_101_neg",
        canister_argument="(-101 : int32)",
        network=network,
    )
    expected_response = "(-101 : int32)"
    assert response == expected_response


def test__roundtrip_int32_1_000_000_001_neg(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_int32_1_000_000_001_neg",
        canister_argument="(-1_000_000_001 : int32)",
        network=network,
    )
    expected_response = "(-1_000_000_001 : int32)"
    assert response == expected_response


def test__roundtrip_int64_101(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_int64_101",
        canister_argument="(101 : int64)",
        network=network,
    )
    expected_response = "(101 : int64)"
    assert response == expected_response


def test__roundtrip_int64_1_000_000_000_000_000_001(
    network: str, principal: str
) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_int64_1_000_000_000_000_000_001",
        canister_argument="(1_000_000_000_000_000_001 : int64)",
        network=network,
    )
    expected_response = "(1_000_000_000_000_000_001 : int64)"
    assert response == expected_response


def test__roundtrip_int64_101_neg(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_int64_101_neg",
        canister_argument="(-101 : int64)",
        network=network,
    )
    expected_response = "(-101 : int64)"
    assert response == expected_response


def test__roundtrip_int64_1_000_000_000_000_000_001_neg(
    network: str, principal: str
) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_int64_1_000_000_000_000_000_001_neg",
        canister_argument="(-1_000_000_000_000_000_001 : int64)",
        network=network,
    )
    expected_response = "(-1_000_000_000_000_000_001 : int64)"
    assert response == expected_response


def test__canister_roundtrip_text(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_text",
        canister_argument='("Hello C++ Canister")',
        network=network,
    )
    expected_response = '("Hello C++ Canister")'
    assert response == expected_response


def test__roundtrip_text_to_json_to_text(network: str, principal: str) -> None:
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


def test__roundtrip_text_to_json_to_text__long_message(
    network: str, principal: str
) -> None:
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


def test__roundtrip_text_to_json_to_text__negative(
    network: str, principal: str
) -> None:
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


def test__roundtrip_text_to_json_to_text__negative_2(
    network: str, principal: str
) -> None:
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


def test__roundtrip_reserved(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_reserved",
        canister_argument="(null : reserved)",
        network=network,
    )
    expected_response = "(null : reserved)"
    assert response == expected_response


def test__roundtrip_float32(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_float32",
        canister_argument="(1001.1001 : float32)",
        network=network,
    )
    expected_response = "(1001.1001 : float32)"
    assert response == expected_response


def test__roundtrip_float32_neg(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_float32_neg",
        canister_argument="(-1001.1001 : float32)",
        network=network,
    )
    expected_response = "(-1001.1001 : float32)"
    assert response == expected_response


def test__roundtrip_float64(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_float64",
        canister_argument="(1001.1001 : float64)",
        network=network,
    )
    expected_response = "(1001.1001 : float64)"
    assert response == expected_response


def test__roundtrip_float64_neg(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_float64_neg",
        canister_argument="(-1001.1001 : float64)",
        network=network,
    )
    expected_response = "(-1001.1001 : float64)"
    assert response == expected_response


def test__roundtrip_principal(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_principal",
        canister_argument='(principal "expmt-gtxsw-inftj-ttabj-qhp5s-nozup-n3bbo-k7zvn-dg4he-knac3-lae")',
        network=network,
    )
    expected_response = (
        '(principal "expmt-gtxsw-inftj-ttabj-qhp5s-nozup-n3bbo-k7zvn-dg4he-knac3-lae")'
    )
    assert response == expected_response


def test__caller_is_anonymous_true(
    identity_anonymous: dict[str, str], network: str
) -> None:
    # double check the identity_anonymous fixture worked
    assert identity_anonymous["identity"] == "anonymous"
    assert identity_anonymous["principal"] == "2vxsx-fae"

    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="caller_is_anonymous",
        canister_argument="()",
        network=network,
    )
    expected_response = "(true)"
    assert response == expected_response


def test__caller_is_anonymous_false(
    identity_default: dict[str, str], network: str
) -> None:
    # double check the identity_default fixture worked
    assert identity_default["identity"] == "default"

    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="caller_is_anonymous",
        canister_argument="()",
        network=network,
    )
    expected_response = "(false)"
    assert response == expected_response


def test__roundtrip_vec_nat16(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_vec_nat16",
        canister_argument="(vec { 101 : nat16; 102 : nat16; 103 : nat16 })",
        network=network,
    )
    expected_response = "(vec { 101 : nat16; 102 : nat16; 103 : nat16;})"
    assert response == expected_response


def test__roundtrip_vec_record(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_vec_record",
        canister_argument="4449444c026d016c02f1fee18d0371cbe4fdc70471010003034831560348314e034832560348324e034833560348334e",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c086c02f1fee18d0301cbe4fdc704016d716c006c02f1fee18d0371cbe4fdc704716d036c02f1fee18d0371cbe4fdc704716c02f1fee18d0371cbe4fdc704716c02f1fee18d0371cbe4fdc70471010403034831560348314e034832560348324e034833560348334e"
    assert response == expected_response


def test__roundtrip_vec_record_2(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_vec_record",
        canister_argument="4449444c086c02f1fee18d0301cbe4fdc704016d716c006c02f1fee18d0371cbe4fdc704716d036c02f1fee18d0371cbe4fdc704716c02f1fee18d0371cbe4fdc704716c02f1fee18d0371cbe4fdc70471010403034831560348314e034832560348324e034833560348334e",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c086c02f1fee18d0301cbe4fdc704016d716c006c02f1fee18d0371cbe4fdc704716d036c02f1fee18d0371cbe4fdc704716c02f1fee18d0371cbe4fdc704716c02f1fee18d0371cbe4fdc70471010403034831560348314e034832560348324e034833560348334e"
    assert response == expected_response


def test__roundtrip_vec_all(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_vec_all",
        canister_argument="4449444c0f6d7e6d7d6d7b6d7a6d796d786d7c6d776d766d756d746d736d726d716d680f000102030405060708090a0b0c0d0e0201000365666703656667036500660067000365000000660000006700000003650000000000000066000000000000006700000000000000039b7f9a7f997f039b9a99039bff9aff99ff039bffffff9affffff99ffffff039bffffffffffffff9affffffffffffff99ffffffffffffff03ae4781bf5c8f82bf0ad783bf03295c8fc2f528f0bf52b81e85eb51f0bf7b14ae47e17af0bf030948656c6c6f203130310948656c6c6f203130320948656c6c6f203130330301010001020102011d779590d2cd339802981dfd935d9a3dbb085cafe6ad19b87229a016d602",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c0f6d7e6d7d6d7b6d7a6d796d786d7c6d776d766d756d746d736d726d716d680f000102030405060708090a0b0c0d0e0201000365666703656667036500660067000365000000660000006700000003650000000000000066000000000000006700000000000000039b7f9a7f997f039b9a99039bff9aff99ff039bffffff9affffff99ffffff039bffffffffffffff9affffffffffffff99ffffffffffffff03ae4781bf5c8f82bf0ad783bf03295c8fc2f528f0bf52b81e85eb51f0bf7b14ae47e17af0bf030948656c6c6f203130310948656c6c6f203130320948656c6c6f203130330301010001020102011d779590d2cd339802981dfd935d9a3dbb085cafe6ad19b87229a016d602"
    assert response == expected_response


def test__roundtrip_opt_nat(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_opt_nat",
        canister_argument="(opt (101 : nat))",
        network=network,
    )
    expected_response = "(opt (101 : nat))"
    assert response == expected_response


def test__roundtrip_opt_nat_nat_1(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_opt_nat_nat",
        canister_argument="(opt (101 : nat), 102 : nat)",
        network=network,
    )
    expected_response = "(102 : nat)"
    assert response == expected_response


def test__roundtrip_opt_nat_nat_2(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_opt_nat_nat",
        # canister_argument="(opt (null : null), 102 : nat)", # dfx complains
        canister_argument="4449444c016e7f02007d0166",
        canister_input="raw",
        canister_output="idl",
        network=network,
    )
    expected_response = "(102 : nat)"
    assert response == expected_response


def test__roundtrip_opt_nat_nat_3(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_opt_nat_nat",
        # canister_argument="(102 : nat)", # dfx complains
        canister_argument="4449444c00017d66",
        canister_input="raw",
        canister_output="idl",
        network=network,
    )
    expected_response = "(102 : nat)"
    assert response == expected_response


def test__roundtrip_opt_nat_no_value(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_opt_nat_no_value",
        canister_argument="(null)",
        network=network,
    )
    expected_response = "(null)"
    assert response == expected_response


def test__roundtrip_opt_nat16(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_opt_nat16",
        canister_argument="(opt (101 : nat16))",
        network=network,
    )
    expected_response = "(opt (101 : nat16))"
    assert response == expected_response


def test__roundtrip_opt_nat16_no_value(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_opt_nat16_no_value",
        canister_argument="(null)",
        network=network,
    )
    expected_response = "(null)"
    assert response == expected_response


# The raw bytes returned represent an opt : nat16 without a value.
# There is no IDL representation for this.
# didc decodes it to '(null)'
def test__roundtrip_opt_nat16_no_value_raw(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_opt_nat16_no_value",
        canister_argument="4449444c016e7a010000",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c016e7a010000"
    assert response == expected_response


def test__roundtrip_opt_record(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_opt_record",
        canister_argument="4449444c026e016c02b79cba840871b89cba84087a0100010568656c6c6f1000",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = (
        "4449444c036c02b79cba840871b89cba84087a6c006e000102010568656c6c6f1000"
    )
    assert response == expected_response


def test__roundtrip_opt_vec_nat16(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_opt_vec_nat16",
        canister_argument="4449444c026e016d7a01000103650066006700",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c026d7a6e0001010103650066006700"
    assert response == expected_response


def test__roundtrip_opt_vec_nat16_2(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_opt_vec_nat16",
        canister_argument="4449444c026d7a6e0001010103650066006700",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c026d7a6e0001010103650066006700"
    assert response == expected_response


def test__roundtrip_opt_record_no_value(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_opt_record_no_value",
        canister_argument="4449444c0000",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c0000"
    assert response == expected_response


def test__roundtrip_opt_variant_ok(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_opt_variant_ok",
        canister_argument="4449444c026e016b01bc8a017f01000100",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c036b01bc8a017f6b006e0001020100"
    assert response == expected_response


def test__roundtrip_opt_variant_ok_no_value(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_opt_variant_ok_no_value",
        canister_argument="4449444c0000",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c0000"
    assert response == expected_response


def test__roundtrip_opt_variant_err(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_opt_variant_err",
        canister_argument="4449444c026e016b01c5fed2017101000100054572726f72",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c036b01c5fed201716b006e0001020100054572726f72"
    assert response == expected_response


def test__roundtrip_opt_variant_err_no_value(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_opt_variant_err_no_value",
        canister_argument="4449444c0000",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c0000"
    assert response == expected_response


def test__roundtrip_opt_record_variant(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_opt_record_variant",
        canister_argument="4449444c036e016c03b79cba840871b89cba840802b99cba8408716b01fc9cba84087c0100010568656c6c6f001003627965",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c046b01fc9cba84087c6c03b79cba840871b89cba840800b99cba8408716c006e010103010568656c6c6f001003627965"
    assert response == expected_response


# def test__roundtrip_opt_record_variant_no_value_1(
# network: str, principal: str
# ) -> None:
# response = call_canister_api(
# dfx_json_path=DFX_JSON_PATH,
# canister_name=CANISTER_NAME,
# canister_method="roundtrip_opt_record_variant_no_value",
# canister_argument="4449444c026e016c01b69cba8408710100010757726f6e674964",
# canister_input="raw",
# canister_output="raw",
# network=network,
# )
# expected_response = "4449444c0000"
# assert response == expected_response


# def test__roundtrip_opt_record_variant_no_value_2(
# network: str, principal: str
# ) -> None:
# response = call_canister_api(
# dfx_json_path=DFX_JSON_PATH,
# canister_name=CANISTER_NAME,
# canister_method="roundtrip_opt_record_variant_no_value",
# canister_argument="4449444c036e016c03b79cba840871b89cba840802b99cba8408716b01b4bcb8e10f7c0100010568656c6c6f001003627965",
# canister_input="raw",
# canister_output="raw",
# network=network,
# )
# expected_response = "4449444c0000"
# assert response == expected_response


def test__roundtrip_opt_vec_record(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_opt_vec_record",
        canister_argument="4449444c036e016d026c02b79cba840871b89cba840871010001030668656c6c6f3104627965310668656c6c6f3204627965320668656c6c6f330462796533",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c096c02b79cba840801b89cba8408016d716c006c02b79cba840871b89cba8408716d036c02b79cba840871b89cba8408716c02b79cba840871b89cba8408716c02b79cba840871b89cba8408716e04010801030668656c6c6f3104627965310668656c6c6f3204627965320668656c6c6f330462796533"
    assert response == expected_response


def test__roundtrip_opt_vec_record_2(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_opt_vec_record",
        canister_argument="4449444c096c02b79cba840801b89cba8408016d716c006c02b79cba840871b89cba8408716d036c02b79cba840871b89cba8408716c02b79cba840871b89cba8408716c02b79cba840871b89cba8408716e04010801030668656c6c6f3104627965310668656c6c6f3204627965320668656c6c6f330462796533",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c096c02b79cba840801b89cba8408016d716c006c02b79cba840871b89cba8408716d036c02b79cba840871b89cba8408716c02b79cba840871b89cba8408716c02b79cba840871b89cba8408716e04010801030668656c6c6f3104627965310668656c6c6f3204627965320668656c6c6f330462796533"
    assert response == expected_response


def test__roundtrip_opt_all_raw(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_opt_all",
        canister_argument="4449444c0f6e7e6e7d6e7b6e7a6e796e786e7c6e776e766e756e746e736e726e716e681f0000000101020203030404050506060707080809090a0a0b0b0c0c0d0d0e0e01010100000165000166000167000001680000000001690000000000000000019b7f00019a000199ff000198ffffff000197ffffffffffffff0001ae4781bf000152b81e85eb51f0bf00010948656c6c6f203130310001011d779590d2cd339802981dfd935d9a3dbb085cafe6ad19b87229a016d60200",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c0f6e7e6e7d6e7b6e7a6e796e786e7c6e776e766e756e746e736e726e716e681f0000000101020203030404050506060707080809090a0a0b0b0c0c0d0d0e0e01010100000165000166000167000001680000000001690000000000000000019b7f00019a000199ff000198ffffff000197ffffffffffffff0001ae4781bf000152b81e85eb51f0bf00010948656c6c6f203130310001011d779590d2cd339802981dfd935d9a3dbb085cafe6ad19b87229a016d60200"
    assert response == expected_response


def test__roundtrip_record(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_record",
        canister_argument='(record {"name" = "C++ Developer"; "secret float64" = 0.01 : float64; "secret int" = 11 : int;})',
        network=network,
    )
    # Windows & Linux behave slightly different with quotation. Just strip it out before assert
    expected_response = '(record { "secret float64" = 0.01 : float64; greeting = "Hello C++ Developer!"; "secret int" = 11 : int; message = "Your secret numbers are:";})'
    assert response.replace('"', "") == expected_response.replace('"', "")


def test__roundtrip_record_record_record_1(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_record_record_record",
        canister_argument="4449444c036c01b2c39bb804016c01b3c39bb804026c01b4c39bb8047101000676616c756531",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = (
        "4449444c036c01b4c39bb804716c01b3c39bb804006c01b2c39bb8040101020676616c756531"
    )
    assert response == expected_response


def test__roundtrip_record_record_record_2(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_record_record_record",
        canister_argument="4449444c036c01b4c39bb804716c01b3c39bb804006c01b2c39bb8040101020676616c756531",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = (
        "4449444c036c01b4c39bb804716c01b3c39bb804006c01b2c39bb8040101020676616c756531"
    )
    assert response == expected_response


def test__roundtrip_record_record_record_3(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_record_record_record",
        canister_argument="4449444c046c006c01b4c39bb804716c01b3c39bb804016c01b2c39bb8040201030676616c756531",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = (
        "4449444c036c01b4c39bb804716c01b3c39bb804006c01b2c39bb8040101020676616c756531"
    )
    assert response == expected_response


def test__roundtrip_record_vec_text_1(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_record_vec_text",
        canister_argument="4449444c026c02e287dcfd0401c89f92b409016d710100020348315603483256020348314e0348324e",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c026c02e287dcfd0401c89f92b409016d710100020348315603483256020348314e0348324e"
    assert response == expected_response


def test__roundtrip_record_vec_text_2(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_record_vec_text",
        canister_argument="4449444c036c006d716c02e287dcfd0401c89f92b409010102020348315603483256020348314e0348324e",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c026c02e287dcfd0401c89f92b409016d710100020348315603483256020348314e0348324e"
    assert response == expected_response


# ------------------------------------------------------------------------
# Record with Opt fields
def test__roundtrip_record_opt_1(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_record_opt",
        canister_argument="4449444c036c02b79cba840801b99cba8408026e7a6e71027b0001011000010568656c6c6f",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = (
        "4449444c036c02b79cba840802b99cba8408016e716e7a0100011000010568656c6c6f"
    )
    assert response == expected_response


def test__roundtrip_record_opt_2a(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_record_opt",
        canister_argument="4449444c026c02b79cba840801b99cba8408016e7f027b00020101",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = (
        "4449444c036c02b79cba840802b99cba8408016e716e7a0100011000010568656c6c6f"
    )
    assert response == expected_response


def test__roundtrip_record_opt_2b(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_record_opt",
        canister_argument="4449444c016c00027b0002",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = (
        "4449444c036c02b79cba840802b99cba8408016e716e7a0100011000010568656c6c6f"
    )
    assert response == expected_response


def test__roundtrip_record_opt_3a(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_record_opt",
        canister_argument="4449444c036c02b79cba840801b99cba8408026e7a6e7f027b000301100001",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = (
        "4449444c036c02b79cba840802b99cba8408016e716e7a0100011000010568656c6c6f"
    )
    assert response == expected_response


def test__roundtrip_record_opt_3b(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_record_opt",
        canister_argument="4449444c026c01b79cba8408016e7a027b0003011000",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = (
        "4449444c036c02b79cba840802b99cba8408016e716e7a0100011000010568656c6c6f"
    )
    assert response == expected_response


def test__roundtrip_record_opt_4a(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_record_opt",
        canister_argument="4449444c036c02b79cba840801b99cba8408026e7f6e71027b000401010568656c6c6f",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = (
        "4449444c036c02b79cba840802b99cba8408016e716e7a0100011000010568656c6c6f"
    )
    assert response == expected_response


def test__roundtrip_record_opt_4b(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_record_opt",
        canister_argument="4449444c026c01b99cba8408016e71027b0004010568656c6c6f",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = (
        "4449444c036c02b79cba840802b99cba8408016e716e7a0100011000010568656c6c6f"
    )
    assert response == expected_response


# ------------------------------------------------------------------------
# Record with Opt Variant fields
def test__roundtrip_record_opt_variant_1(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_record_opt_variant",
        canister_argument="4449444c056c02b79cba840801b99cba8408036e026b01bc8a017f6e046b01c5fed20171027b000101000100054572726f72",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c066c02b79cba840805b99cba8408036b01c5fed201716b006e016b01bc8a017f6e04010001000100054572726f72"
    assert response == expected_response


def test__roundtrip_record_opt_variant_2a(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_record_opt_variant",
        canister_argument="4449444c026c02b79cba840801b99cba8408016e7f027b00020101",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c066c02b79cba840805b99cba8408036b01c5fed201716b006e016b01bc8a017f6e04010001000100054572726f72"
    assert response == expected_response


def test__roundtrip_record_opt_variant_2b(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_record_opt_variant",
        canister_argument="4449444c016c00027b0002",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c066c02b79cba840805b99cba8408036b01c5fed201716b006e016b01bc8a017f6e04010001000100054572726f72"
    assert response == expected_response


def test__roundtrip_record_opt_variant_3a(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_record_opt_variant",
        canister_argument="4449444c046c02b79cba840801b99cba8408036e026b01bc8a017f6e7f027b0003010001",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c066c02b79cba840805b99cba8408036b01c5fed201716b006e016b01bc8a017f6e04010001000100054572726f72"
    assert response == expected_response


def test__roundtrip_record_opt_variant_3b(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_record_opt_variant",
        canister_argument="4449444c036c01b79cba8408016e026b01bc8a017f027b00030100",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c066c02b79cba840805b99cba8408036b01c5fed201716b006e016b01bc8a017f6e04010001000100054572726f72"
    assert response == expected_response


def test__roundtrip_record_opt_variant_4a(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_record_opt_variant",
        canister_argument="4449444c046c02b79cba840801b99cba8408026e7f6e036b01c5fed20171027b0004010100054572726f72",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c066c02b79cba840805b99cba8408036b01c5fed201716b006e016b01bc8a017f6e04010001000100054572726f72"
    assert response == expected_response


def test__roundtrip_record_opt_variant_4b(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_record_opt_variant",
        canister_argument="4449444c036c01b99cba8408016e026b01c5fed20171027b00040100054572726f72",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c066c02b79cba840805b99cba8408036b01c5fed201716b006e016b01bc8a017f6e04010001000100054572726f72"
    assert response == expected_response


# ------------------------------------------------------------------------
# Record with Vec Record field (headers of http_request)
def test__roundtrip_record_vec_record_1(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_record_vec_record",
        canister_argument="4449444c036c01c6a4a19806016d026c02f1fee18d0371cbe4fdc70471010003034831560348314e034832560348324e034833560348334e",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c096c02f1fee18d0301cbe4fdc704016d716c006c02f1fee18d0371cbe4fdc704716d036c02f1fee18d0371cbe4fdc704716c02f1fee18d0371cbe4fdc704716c02f1fee18d0371cbe4fdc704716c01c6a4a1980604010803034831560348314e034832560348324e034833560348334e"
    assert response == expected_response


def test__roundtrip_record_vec_record_2(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_record_vec_record",
        canister_argument="4449444c096c02f1fee18d0301cbe4fdc704016d716c006c02f1fee18d0371cbe4fdc704716d036c02f1fee18d0371cbe4fdc704716c02f1fee18d0371cbe4fdc704716c02f1fee18d0371cbe4fdc704716c01c6a4a1980604010803034831560348314e034832560348324e034833560348334e",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c096c02f1fee18d0301cbe4fdc704016d716c006c02f1fee18d0371cbe4fdc704716d036c02f1fee18d0371cbe4fdc704716c02f1fee18d0371cbe4fdc704716c02f1fee18d0371cbe4fdc704716c01c6a4a1980604010803034831560348314e034832560348324e034833560348334e"
    assert response == expected_response


# Record with Vec Record field (headers of http_request)
def test__roundtrip_http_headers(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_http_headers",
        canister_argument="4449444c036c01c6a4a19806016d026c020071017101000704686f73742735756772762d7a716161612d61616161672d6163666e612d6361692e7261772e696370302e696f09782d7265616c2d69700d32312e39382e3234312e3234350f782d666f727761726465642d666f720d32312e39382e3234312e32343511782d666f727761726465642d70726f746f0568747470730c782d726571756573742d69642464396234316462342d346539372d383237362d313466312d3664623764303037623537640a757365722d6167656e740b6375726c2f372e38312e3006616363657074032a2f2a",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c0d6c02000101016d716c006c02007101716d036c02007101716c02007101716c02007101716c02007101716c02007101716c02007101716c02007101716c01c6a4a1980604010c0704686f73742735756772762d7a716161612d61616161672d6163666e612d6361692e7261772e696370302e696f09782d7265616c2d69700d32312e39382e3234312e3234350f782d666f727761726465642d666f720d32312e39382e3234312e32343511782d666f727761726465642d70726f746f0568747470730c782d726571756573742d69642464396234316462342d346539372d383237362d313466312d3664623764303037623537640a757365722d6167656e740b6375726c2f372e38312e3006616363657074032a2f2a"
    assert response == expected_response


# ------------------------------------------------------------------------
# Variant
def test__roundtrip_variant_ok(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_variant_ok",
        canister_argument="4449444c016b01bc8a017f010000",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c016b01bc8a017f010000"
    assert response == expected_response


def test__roundtrip_variant_err(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_variant_err",
        canister_argument="4449444c016b01c5fed20171010000054572726f72",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c016b01c5fed20171010000054572726f72"
    assert response == expected_response


def test__roundtrip_variant_1(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_variant",
        canister_argument="4449444c046b01bc8a017f6b01c5fed2017a6b01bc8a01716b01c5fed2017106000102010203000094010008416c6c20676f6f6400f4010008416c6c20676f6f6400054572726f72",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c066b02bc8a017fc5fed2017a6b02bc8a017fc5fed2017a6b02bc8a0171c5fed2017a6b02bc8a0171c5fed2017a6b02bc8a0171c5fed201716b02bc8a0171c5fed2017106000102030405000194010008416c6c20676f6f6401f4010008416c6c20676f6f6401054572726f72"
    assert response == expected_response


def test__roundtrip_variant_2(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_variant",
        canister_argument="4449444c036b02bc8a017fc5fed2017a6b02bc8a0171c5fed2017a6b02bc8a0171c5fed2017106000001010202000194010008416c6c20676f6f6401f4010008416c6c20676f6f6401054572726f72",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c066b02bc8a017fc5fed2017a6b02bc8a017fc5fed2017a6b02bc8a0171c5fed2017a6b02bc8a0171c5fed2017a6b02bc8a0171c5fed201716b02bc8a0171c5fed2017106000102030405000194010008416c6c20676f6f6401f4010008416c6c20676f6f6401054572726f72"
    assert response == expected_response


def test__roundtrip_variant_3(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_variant",
        canister_argument="4449444c066b02bc8a017fc5fed2017a6b02bc8a017fc5fed2017a6b02bc8a0171c5fed2017a6b02bc8a0171c5fed2017a6b02bc8a0171c5fed201716b02bc8a0171c5fed2017106000102030405000194010008416c6c20676f6f6401f4010008416c6c20676f6f6401054572726f72",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c066b02bc8a017fc5fed2017a6b02bc8a017fc5fed2017a6b02bc8a0171c5fed2017a6b02bc8a0171c5fed2017a6b02bc8a0171c5fed201716b02bc8a0171c5fed2017106000102030405000194010008416c6c20676f6f6401f4010008416c6c20676f6f6401054572726f72"
    assert response == expected_response


# ------------------------------------------------------------------------
# Variant with Opt field
def test__roundtrip_variant_opt_1(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_variant_opt",
        canister_argument="4449444c026b01c5fed201016e71027b00010001054572726f72",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c026b01c5fed201016e7101000001054572726f72"
    assert response == expected_response


def test__roundtrip_variant_opt_2a(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_variant_opt",
        canister_argument="4449444c026b01c5fed201016e7f027b00020001",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c026b01c5fed201016e7101000001054572726f72"
    assert response == expected_response


# ----------------------------------------------------------------------------------
# Forward Compatibility testing  (Additional Opts)
def test__roundtrip_bool_true_w_opt_1(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_bool_true",
        canister_argument="4449444c016e7f03007e00010101",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c00017e01"
    assert response == expected_response


def test__roundtrip_bool_true_w_opt_2(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_bool_true",
        canister_argument="4449444c026e716e7b03007e01010765787472612031010165",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c00017e01"
    assert response == expected_response


def test__roundtrip_bool_true_w_opt_3(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_bool_true",
        canister_argument="4449444c036e016c02b79cba84087ab89cba8408716e7b03007e02011000026632010165",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c00017e01"
    assert response == expected_response


def test__roundtrip_bool_true_w_opt_4(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_bool_true",
        canister_argument="4449444c046e016b01bc8a017f6e036b01c5fed2017103007e020100010100054572726f72",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c00017e01"
    assert response == expected_response


def test__roundtrip_bool_true_w_opt_5(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_bool_true",
        canister_argument="4449444c046e016b01c5fed201716e036b01bc8a017f03007e020100054572726f72010100",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c00017e01"
    assert response == expected_response


def test__roundtrip_bool_true_w_opt_6(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_bool_true",
        canister_argument="4449444c086e016c01b79cba8408716e036b01c5fed201716e056b01bc8a017f6e076c01b89cba8408710500027e0406010568656c6c6f0100054572726f720101000103627965",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c00017e01"
    assert response == expected_response


def test__roundtrip_bool_true_w_opt_7(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_bool_true",
        canister_argument="4449444c0c6e016c0100026e036b01b79cba8408716e056b01c5fed201716e076b01bc8a017f6e096c01000a6e0b6b01b89cba8408710500047e06080101000568656c6c6f0100054572726f7201010001010003627965",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c00017e01"
    assert response == expected_response


def test__roundtrip_bool_true_w_opt_8(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_bool_true",
        canister_argument="4449444c066e016b01c5fed201026e036b01c5fed201716e056b01bc8a017f04007e040001000100054572726f7201010001000100054572726f72",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c00017e01"
    assert response == expected_response


def test__roundtrip_bool_true_w_opt_9(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_bool_true",
        canister_argument="4449444c086e016c0100026e036b01c5fed201046e056b01c5fed201716e076b01bc8a017f0500047e06000101000100054572726f720100054572726f720101000101000100054572726f72",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c00017e01"
    assert response == expected_response


def test__roundtrip_bool_true_w_opt_10(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_bool_true",
        canister_argument="4449444c026e016d7a03007e000103650066006700010103650066006700",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c00017e01"
    assert response == expected_response


def test__roundtrip_bool_true_w_opt_11(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_bool_true",
        canister_argument="4449444c036e016d026c02b79cba840871b89cba84087103007e0001030668656c6c6f3104627965310668656c6c6f3204627965320668656c6c6f3204627965330101030668656c6c6f3104627965310668656c6c6f3204627965320668656c6c6f320462796533",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c00017e01"
    assert response == expected_response


def test__roundtrip_bool_true_w_opt_12(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_bool_true",
        canister_argument="4449444c046e016d026c03b79cba840871b89cba840803b99cba8408716b01fc9cba84087c03007e0001030668656c6c6f31000104627965310668656c6c6f32000204627965320668656c6c6f32000204627965320101030668656c6c6f31000104627965310668656c6c6f32000204627965320668656c6c6f3200020462796532",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c00017e01"
    assert response == expected_response


def test__roundtrip_record_opt_1_fwc_a(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_record_opt",
        canister_argument="4449444c036c05b69cba840801b79cba840802b89cba840801b99cba840801ba9cba8408016e716e7a027b000101097374617274206f7074011000010a6d6964646c65206f7074010568656c6c6f0107656e64206f7074",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = (
        "4449444c036c02b79cba840802b99cba8408016e716e7a0100011000010568656c6c6f"
    )
    assert response == expected_response


def test__roundtrip_record_opt_1_fwc_b(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_record_opt",
        canister_argument="4449444c046c05b69cba840801b79cba840802b89cba840801b99cba840803ba9cba8408016e7f6e7a6e71027b00010101100001010568656c6c6f01",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = (
        "4449444c036c02b79cba840802b99cba8408016e716e7a0100011000010568656c6c6f"
    )
    assert response == expected_response


def test__roundtrip_vec_record_w_opts(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_vec_record",
        canister_argument="4449444c036d016c05e1bde70202f1fee18d03718094ebdc0302cbe4fdc70471b49f92b409026e710100030109626567696e206f707403483156010a6d6964646c65206f70740348314e0107656e64206f70740109626567696e206f707403483256010a6d6964646c65206f70740348324e0107656e64206f70740109626567696e206f707403483356010a6d6964646c65206f70740348334e0107656e64206f7074",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c086c02f1fee18d0301cbe4fdc704016d716c006c02f1fee18d0371cbe4fdc704716d036c02f1fee18d0371cbe4fdc704716c02f1fee18d0371cbe4fdc704716c02f1fee18d0371cbe4fdc70471010403034831560348314e034832560348324e034833560348334e"
    assert response == expected_response


# ----------------------------------------------------------------------------------
# Trap testing
#
# Verify that a Deserialization traps if the number of arguments is wrong
def test__trap_wrong_number_of_args(network: str, principal: str) -> None:
    """Verify that a Deserialization traps if the number of arguments is wrong"""
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="trap_wrong_number_of_args",
        canister_argument="4449444c00027e7e0101",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    assert "Failed call to api" in response
    assert "trapped explicitly" in response
    assert (
        "ERROR: Done with all the expected args, but there is an additional non Opt arg on the wire."
        in response
    )


# Verify that a canister traps if from_wire is called more than once
def test__trap_multiple_calls_from_wire(network: str, principal: str) -> None:
    """Verify that api traps if from_wire is called more than once"""
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="trap_multiple_calls_from_wire",
        canister_argument="4449444c00017e01",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    assert "Failed call to api" in response
    assert "trapped explicitly" in response
    assert (
        "ERROR: The canister is calling ic_api.from_wire() more than once." in response
    )


# Verify that a canister traps if to_wire is called more than once
def test__trap_multiple_calls_to_wire(network: str, principal: str) -> None:
    """Verify that api traps if to_wire is called more than once"""
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="trap_multiple_calls_to_wire",
        canister_argument="4449444c00017e01",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    assert "Failed call to api" in response
    assert "trapped explicitly" in response
    assert "ERROR: The canister is calling ic_api.to_wire() more than once." in response


# Verify that a record traps if one of the records's id (hash) on wire does not match expected
def test__trap_roundtrip_record(network: str, principal: str) -> None:
    """Verify that a record traps if one of the records's id (hash) on wire does not match expected"""
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_record",
        canister_argument="4449444c016c03e3d2a2c10172cbe4fdc70471bfce83fe077c01007b14ae47e17a843f0d432b2b20446576656c6f7065720b",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    assert "Failed call to api" in response
    assert "trapped explicitly" in response
