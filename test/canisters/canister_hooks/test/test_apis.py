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
    """The built-in fix and the hook each kept their own backup.

    This canister is built twice, once per config, and `--to-compile all`
    wipes build/ each time, so the two cases are cleanly separated:

      * icpp.toml        - fix on + post_wasm_function. The hook asserts the
                           ordering contract at BUILD time (it raises, failing
                           the build, if it does not already see a fixed
                           wasm). Here we prove the two backups coexist: the
                           built-in `_before_opt_internal` holds the
                           PRE-optimize wasm, the hook's own `_before_opt`
                           holds the POST-fix wasm it received. Before the
                           suffixes were split these were one file, and the
                           hook's copy silently destroyed ours.
      * icpp-no-fix.toml - fix_globals_limit = false and no hook. Neither
                           backup may exist.
    """
    wasm_path = (BUILD_PATH / "build/my_canister.wasm").resolve()
    internal_backup = (
        BUILD_PATH / "build/my_canister_before_opt_internal.wasm"
    ).resolve()
    hook_backup = (BUILD_PATH / "build/my_canister_before_opt.wasm").resolve()
    hook_marker = (BUILD_PATH / "build/post_wasm_function_ran.marker").resolve()

    assert wasm_path.is_file(), f"no wasm at {wasm_path}"

    # The marker says which config produced this build, so each branch can
    # assert the FULL expected state. Inferring the mode from the backups
    # themselves would let a wrongly-enabled fix in icpp-no-fix.toml satisfy
    # the fix-is-on branch and pass.
    if hook_marker.is_file():
        # icpp.toml: fix on + hook.
        assert internal_backup.is_file(), "the built-in backup is missing"
        assert hook_backup.is_file(), "the hook's own backup is missing"
        # Two files, two stages - they must not be the same bytes.
        assert internal_backup.read_bytes() != hook_backup.read_bytes()
        # Ours is the pre-optimize wasm, so it differs from what was deployed.
        assert internal_backup.read_bytes() != wasm_path.read_bytes()
    else:
        # icpp-no-fix.toml: fix_globals_limit = false and no hook, so the step
        # must have been skipped entirely - no backup of either kind.
        assert (
            not internal_backup.exists()
        ), "fix_globals_limit = false but the built-in step still ran"
        assert not hook_backup.exists()
