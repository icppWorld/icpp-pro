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
    expected_response = '(record { secret float64 = 0.01 : float64; greeting = "Hello C++ Developer!"; secret int = 11 : int; message = "Your secret numbers are:";})'
    assert response == expected_response


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


def test__roundtrip_variant(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="roundtrip_variant",
        canister_argument="4449444c046b01bc8a017f6b01c5fed2017a6b01bc8a01716b01c5fed2017106000102010203000094010008416c6c20676f6f6400f4010008416c6c20676f6f6400054572726f72",
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    expected_response = "4449444c036b02bc8a017fc5fed2017a6b02bc8a0171c5fed2017a6b02bc8a0171c5fed2017106000001010202000194010008416c6c20676f6f6401f4010008416c6c20676f6f6401054572726f72"
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
    assert "ERROR: wrong number of arguments on wire." in response


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
    assert (
        "ERROR: the hashed id for the Record field at index 0 is wrong on the wire."
        in response
    )
