"""Test canister APIs

First deploy the canister, then run:

$ pytest --network=[local/ic] test/test_apis.py

After a fresh install only canister_init has run. Upgrade the canister in
place and run test/test_after_upgrade.py to check the upgrade hooks.
"""

# pylint: disable=missing-function-docstring

from .upgrade_record import get_upgrade_record


def test__record_after_install(network: str, principal: str) -> None:
    record = get_upgrade_record(network)
    assert record["init_count"] == 1
    assert record["pre_upgrade_count"] == 0
    assert record["post_upgrade_count"] == 0
    # The deploying identity is the caller of canister_init
    assert record["init_caller"] == principal
    assert record["init_self"] != ""
