"""Test canister APIs

First deploy the canister, then run:

$ pytest --network=local test/test_apis.py

After a fresh install only canister_init has run. Upgrade the canister in
place and run test/test_after_upgrade.py to check the upgrade hooks.
"""

# pylint: disable=missing-function-docstring

from .upgrade_record import assert_file, get_upgrade_record, number, write_file


def test__record_after_install(network: str, principal: str) -> None:
    record = get_upgrade_record(network)
    assert record["init_count"] == 1
    assert record["pre_upgrade_count"] == 0
    assert record["post_upgrade_count"] == 0
    # The deploying identity is the caller of canister_init
    assert record["init_caller"] == principal
    assert record["init_self"] != ""
    # An installed canister has been through at least one code install
    assert number(record, "init_version") >= 1


def test__files_before_upgrade(network: str) -> None:
    # Files that test_after_upgrade.py reads back: one of several MiB, and
    # one overwritten in place by a smaller file
    write_file(network, "kept.bin", 5)
    write_file(network, "shrunk.bin", 8)
    write_file(network, "shrunk.bin", 1)
    assert_file(network, "kept.bin", 5)
    assert_file(network, "shrunk.bin", 1)
