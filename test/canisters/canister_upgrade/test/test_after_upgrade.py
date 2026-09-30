"""Test the upgrade hooks, after upgrading the canister in place

First deploy the canister and run test/test_apis.py, then upgrade it with
`icp deploy --mode upgrade` and run:

$ pytest --network=[local/ic] test/test_after_upgrade.py

canister_pre_upgrade and canister_post_upgrade each construct an IC_API. A
pre_upgrade that traps aborts the upgrade, so these counts prove that
constructing IC_API in pre_upgrade does not trap on a real replica.
"""

# pylint: disable=missing-function-docstring

from .upgrade_record import get_upgrade_record


def test__record_after_upgrade(network: str, principal: str) -> None:
    record = get_upgrade_record(network)
    assert record["init_count"] == 1, "canister_init must not run on an upgrade"
    assert record["pre_upgrade_count"] == 1, (
        "pre_upgrade did not run - the upgrade was skipped "
        "(unchanged wasm hash?) or never happened"
    )
    assert record["post_upgrade_count"] == 1, "post_upgrade did not run"
    # The identity that upgrades the canister is the caller of both hooks
    assert record["pre_upgrade_caller"] == principal
    assert record["post_upgrade_caller"] == principal
    # The canister's own id does not change across an upgrade
    assert record["init_self"] != ""
    assert record["pre_upgrade_self"] == record["init_self"]
    assert record["post_upgrade_self"] == record["init_self"]
