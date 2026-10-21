"""Test the upgrade hooks, after upgrading the canister in place

First deploy the canister and run test/test_apis.py, then upgrade it with
`icp deploy --mode upgrade` and run:

$ pytest --network=[local/ic] test/test_after_upgrade.py

canister_pre_upgrade and canister_post_upgrade each construct an IC_API. A
pre_upgrade that traps aborts the upgrade, so these counts prove that
constructing IC_API in pre_upgrade does not trap on a real replica.
"""

# pylint: disable=missing-function-docstring

from .upgrade_record import assert_file, get_upgrade_record, number, write_file


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
    # pre_upgrade runs on the old code, before the IC counts the upgrade;
    # post_upgrade runs on the new code, after it
    pre_upgrade_version = number(record, "pre_upgrade_version")
    assert pre_upgrade_version >= number(record, "init_version")
    assert number(record, "post_upgrade_version") == pre_upgrade_version + 1


def test__files_after_upgrade(network: str) -> None:
    # The files written before the upgrade, also by a canister built with the
    # previous icpp-pro release, read back intact
    assert_file(network, "kept.bin", 5)
    assert_file(network, "shrunk.bin", 1)


def test__file_overwritten_by_a_smaller_one(network: str) -> None:
    # A file overwritten in place by a smaller one reads like a new file of
    # that size: the read does not walk over the chunks of the larger file
    # that was there before
    write_file(network, "overwritten.bin", 16)
    write_file(network, "overwritten.bin", 1)
    write_file(network, "fresh.bin", 1)
    overwritten = assert_file(network, "overwritten.bin", 1)
    fresh = assert_file(network, "fresh.bin", 1)
    ratio = int(overwritten["instructions"]) / int(fresh["instructions"])
    assert ratio < 2, f"the read of the overwritten file costs {ratio:.1f}x"
