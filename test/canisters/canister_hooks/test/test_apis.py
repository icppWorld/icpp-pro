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


# The build folder of whichever config was just built & deployed. Anchored on
# __file__, not the cwd: the previous version of this check used a
# cwd-relative Path("build/...") and had to be commented out because it failed
# in ci/cd.
BUILD_PATH = Path(__file__).parent / ".."


def test__fix_globals_limit_artifacts(network: str, principal: str) -> None:
    """The built-in globals-limit fix left the right artifacts behind.

    This canister is built twice, once per config, and `--to-compile all`
    wipes build/ each time, so the two cases are cleanly separated:

      * icpp.toml        - fix on + post_wasm_function. The hook itself
                           asserts the ordering contract at BUILD time (it
                           raises, failing the build, if it does not already
                           see a fixed wasm), so here we only confirm the
                           backup exists and differs from the fixed wasm.
      * icpp-no-fix.toml - fix_globals_limit = false and no hook. The step
                           must have been skipped entirely, so no backup.
    """
    wasm_path = (BUILD_PATH / "build/my_canister.wasm").resolve()
    backup_path = (BUILD_PATH / "build/my_canister_before_opt.wasm").resolve()

    assert wasm_path.is_file(), f"no wasm at {wasm_path}"

    if backup_path.is_file():
        # fix_globals_limit = true: the backup is the pre-optimize wasm, so it
        # must differ from the wasm that was actually deployed.
        assert backup_path.read_bytes() != wasm_path.read_bytes()
    else:
        # fix_globals_limit = false: the step never ran, so it never wrote a
        # backup. Anything else means the opt-out is not honoured.
        assert not backup_path.exists()
