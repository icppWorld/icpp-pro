"""Run the Candid specification's test suite on a deployed canister

First deploy the canister, then run:

$ pytest --network=[local/ic]

Every assert of every suite file must pass, and the methods for the recursive
type List must send and receive didc's bytes.
"""

# pylint: disable=missing-function-docstring, unused-import, wildcard-import, unused-wildcard-import

from pathlib import Path

import pytest
from icpp.smoketest import call_canister_api

# Path to the icp.yaml file
ICP_YAML_PATH = Path(__file__).parent / "../icp.yaml"

# Canister in the icp.yaml file we want to test
CANISTER_NAME = "my_canister"

# The suite files, with their number of asserts
SUITES = {
    "official/construct.test.did": 169,
    "official/overshoot.test.did": 10,
    "official/prim.test.did": 168,
    "official/reference.test.did": 50,
    "official/spacebomb.test.did": 17,
    "official/subtypes.test.did": 58,
    "icpp/encode.test.did": 21,
    "rust/compatibility.test.did": 28,
    "rust/encode.test.did": 82,
    "rust/header_len.test.did": 10,
    "rust/number.test.did": 41,
    "rust/parse_value.test.did": 18,
    "rust/serde.test.did": 263,
    "rust/service_compatibility.test.did": 35,
    "rust/skip_optional_queue.test.did": 3,
    "rust/subtype.test.did": 2,
    "rust/type_diagnostics.test.did": 5,
    "rust/type_table_alloc.test.did": 3,
    "rust/types.test.did": 6,
    "rust/value.test.did": 16,
    "motoko/encode.test.did": 21,
    "motoko/idl-any-stable.test.did": 1,
    "motoko/idl-any.test.did": 29,
    "motoko/idl-bad.test.did": 1,
    "motoko/idl-buf-size-bug.test.did": 1,
    "motoko/idl-composite.test.did": 4,
    "motoko/idl-field-escape.test.did": 1,
    "motoko/idl-float32.test.did": 1,
    "motoko/idl-func.test.did": 1,
    "motoko/idl-large-principal.test.did": 6,
    "motoko/idl-nary.test.did": 7,
    "motoko/idl-nat-int.test.did": 4,
    "motoko/idl-option.test.did": 3,
    "motoko/idl-pair.test.did": 1,
    "motoko/idl-principal.test.did": 1,
    "motoko/idl-record.test.did": 5,
    "motoko/idl-shorthand.test.did": 1,
    "motoko/idl-spacebomb.test.did": 12,
    "motoko/idl-sub-ho-neg.test.did": 19,
    "motoko/idl-sub-ho.test.did": 11,
    "motoko/idl-sub-opt-any-record.test.did": 3,
    "motoko/idl-sub-opt-any.test.did": 5,
    "motoko/idl-sub-rec.test.did": 11,
    "motoko/idl-sub-variant.test.did": 2,
    "motoko/idl-sub.test.did": 15,
    "motoko/idl-tuple.test.did": 6,
    "motoko/idl-unexpected-variant.test.did": 1,
    "motoko/idl-variant.test.did": 1,
    "motoko/idl-vector.test.did": 1,
    "motoko/run-deser.test.did": 10,
}


def test__suites(network: str) -> None:
    response = call_canister_api(
        icp_yaml_path=ICP_YAML_PATH,
        canister_name=CANISTER_NAME,
        canister_method="suites",
        network=network,
    )
    for path in SUITES:
        assert f'"{path}"' in response


@pytest.mark.parametrize("path,total", SUITES.items())
def test__run_suite(network: str, path: str, total: int) -> None:
    response = call_canister_api(
        icp_yaml_path=ICP_YAML_PATH,
        canister_name=CANISTER_NAME,
        canister_method="run_suite",
        canister_argument=f'("{path}")',
        network=network,
    )
    assert f"passed = {total} : nat64" in response, response
    assert f"total = {total} : nat64" in response, response
    assert 'failures = ""' in response, response


# type List = opt record { head : int; tail : List }, with bytes of didc
# 0.6.2: `didc encode -d src/my_canister.did -t '(List)'`
LIST_4 = "4449444c026e016c02a0d2aca8047c90eddae704000100010101020103010400"
LIST_0 = "4449444c026e016c02a0d2aca8047c90eddae70400010000"


@pytest.mark.parametrize(
    "argument,expected",
    [
        ("4449444c00017d04", LIST_4),  # (4 : nat)
        ("4449444c00017d00", LIST_0),  # (0 : nat)
    ],
)
def test__list(network: str, argument: str, expected: str) -> None:
    response = call_canister_api(
        icp_yaml_path=ICP_YAML_PATH,
        canister_name=CANISTER_NAME,
        canister_method="list",
        canister_argument=argument,
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    assert response == expected


def test__list_too_deep(network: str) -> None:
    # A List of 64 nests 129 deep, over the limit of 128
    response = call_canister_api(
        icp_yaml_path=ICP_YAML_PATH,
        canister_name=CANISTER_NAME,
        canister_method="list",
        canister_argument="(64 : nat)",
        network=network,
    )
    assert "Values nest more than 128 deep" in response, response


@pytest.mark.parametrize(
    "argument,expected",
    [
        (LIST_4, "4449444c00017c0a"),  # (10 : int)
        (LIST_0, "4449444c00017c00"),  # (0 : int)
    ],
)
def test__list_sum(network: str, argument: str, expected: str) -> None:
    response = call_canister_api(
        icp_yaml_path=ICP_YAML_PATH,
        canister_name=CANISTER_NAME,
        canister_method="list_sum",
        canister_argument=argument,
        canister_input="raw",
        canister_output="raw",
        network=network,
    )
    assert response == expected
