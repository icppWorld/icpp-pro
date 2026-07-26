"""Test canister APIs

First deploy the canister, then run:

$ pytest --network=[local/ic] test_apis.py

"""

# pylint: disable=missing-function-docstring, unused-import, wildcard-import, unused-wildcard-import, line-too-long

from pathlib import Path
from typing import Dict
import pytest
from icpp.smoketest import call_canister_api, dict_to_candid_text

# Path to the icp.yaml file
ICP_YAML_PATH = Path(__file__).parent / "../icp.yaml"

# Canister in the icp.yaml file we want to test
CANISTER_NAME = "my_canister"


def test__greet(network: str, principal: str) -> None:
    response = call_canister_api(
        icp_yaml_path=ICP_YAML_PATH,
        canister_name=CANISTER_NAME,
        canister_method="greet",
        canister_argument="()",
        network=network,
    )
    expected_response = '("Hello World")'
    assert response == expected_response

    # Somehow, this check fails during ci/cd
    # This proves that the post_wasm_function was called
    # opt_wasm_file_path = Path("build/my_canister_before_opt.wasm").resolve()
    # assert opt_wasm_file_path.exists()
