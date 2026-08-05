"""Test canister APIs

First deploy the canister, then run:

$ pytest --network=[local/ic] --identity <name> test_apis.py

`--identity` names the icp identity the tests call as. Instead of passing it
every time, export it once:

$ export ICPP_PRO_TEST_IDENTITY=<name>

Deploy with that same identity, so the caller of a test is the controller.
icpp-pro never uses the machine-wide active identity (`icp identity default`),
because any other process can change it while the tests are running.
"""

# pylint: disable=missing-function-docstring, unused-import, wildcard-import, unused-wildcard-import, line-too-long

from pathlib import Path
from typing import Dict
import pytest
from icpp.smoketest import (
    call_canister_api,
    dict_to_candid_text,
    flatten_candid_text,
)

# Path to the icp.yaml file
ICP_YAML_PATH = Path(__file__).parent / "../icp.yaml"

# Canister in the icp.yaml file we want to test
CANISTER_NAME = "greet"


def test__greet_0(network: str) -> None:
    response = call_canister_api(
        icp_yaml_path=ICP_YAML_PATH,
        canister_name=CANISTER_NAME,
        canister_method="greet_0",
        canister_argument="()",
        network=network,
    )
    expected_response = '("hello!")'
    assert response == expected_response


def test__greet_0_static_lib(network: str) -> None:
    response = call_canister_api(
        icp_yaml_path=ICP_YAML_PATH,
        canister_name=CANISTER_NAME,
        canister_method="greet_0_static_lib",
        canister_argument="()",
        network=network,
    )
    expected_response = '("Hello static library world!")'
    assert response == expected_response


# Run this test with anonymous identity
def test__greet_0_auth_err(identity_anonymous: Dict[str, str], network: str) -> None:
    # The identity_anonymous fixture makes every call in this test run as the
    # anonymous identity, without naming it at the call site.
    assert identity_anonymous["identity"] == "anonymous"
    assert identity_anonymous["principal"] == "2vxsx-fae"

    response = call_canister_api(
        icp_yaml_path=ICP_YAML_PATH,
        canister_name=CANISTER_NAME,
        canister_method="greet_0_auth",
        canister_argument="()",
        network=network,
    )
    expected_response = "(variant { Err = 401 : nat16 })"
    assert response == expected_response


# Run this test with the identity of the pytest session
def test__greet_0_auth_ok(identity_default: Dict[str, str], network: str) -> None:
    # `identity=` names the caller for this one call. identity_default is the
    # identity the session runs as, so the canister greets its principal.
    response = call_canister_api(
        icp_yaml_path=ICP_YAML_PATH,
        canister_name=CANISTER_NAME,
        canister_method="greet_0_auth",
        canister_argument="()",
        network=network,
        identity=identity_default["identity"],
    )
    principal = identity_default["principal"]
    expected_response = (
        f'( variant {{ Ok = record {{ greeting = "Hello {principal}"; }} }}, )'
    )
    assert flatten_candid_text(response) == expected_response


def test__greet_1(network: str) -> None:
    response = call_canister_api(
        icp_yaml_path=ICP_YAML_PATH,
        canister_name=CANISTER_NAME,
        canister_method="greet_1",
        canister_argument="()",
        network=network,
    )
    expected_response = "(2_023 : int)"
    assert response == expected_response


def test__greet_2(network: str, principal: str) -> None:
    response = call_canister_api(
        icp_yaml_path=ICP_YAML_PATH,
        canister_name=CANISTER_NAME,
        canister_method="greet_2",
        canister_argument='("C++ Developer")',
        network=network,
    )
    expected_response = f'( "hello C++ Developer!\\nYour principal is: {principal}", )'
    assert flatten_candid_text(response) == expected_response


def test__greet_3(network: str) -> None:
    response = call_canister_api(
        icp_yaml_path=ICP_YAML_PATH,
        canister_name=CANISTER_NAME,
        canister_method="greet_3",
        canister_argument='(record { "icpp version" = 1 : int; OS = "Linux" : text })',
        network=network,
    )
    expected_response = '( record { "icpp Release Details" = "Version = 1 & Operating System = Linux"; "release year" = 2_023 : int; }, )'
    assert flatten_candid_text(response) == expected_response


def test__greet_4(network: str) -> None:
    response = call_canister_api(
        icp_yaml_path=ICP_YAML_PATH,
        canister_name=CANISTER_NAME,
        canister_method="greet_4",
        canister_argument="(record { 6 = 42 : int; 9 = 43 : int }, record { 7 = 44 : int; 10 = 45 : int })",
        network=network,
    )
    expected_response = (
        '("Hello!", "Your secret numbers are:", 42 : int, 43 : int, 44 : int, 45 : int)'
    )
    assert response == expected_response


def test__greet_json(network: str, principal: str) -> None:
    d = {"name": "AJB"}
    text_in = dict_to_candid_text(d)
    response = call_canister_api(
        icp_yaml_path=ICP_YAML_PATH,
        canister_name=CANISTER_NAME,
        canister_method="greet_json",
        canister_argument=text_in,
        network=network,
    )
    expected_response = '("{\\"greet\\":\\"Hello AJB!\\"}")'
    assert response == expected_response


def test__greet_log_file(network: str) -> None:
    response = call_canister_api(
        icp_yaml_path=ICP_YAML_PATH,
        canister_name=CANISTER_NAME,
        canister_method="greet_log_file",
        canister_argument="()",
        network=network,
    )
    # dfx printed nothing for a unit response, icp prints the empty tuple
    expected_response = "()"
    assert response == expected_response
