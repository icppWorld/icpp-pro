"""Test canister APIs for canister_timers.

   First deploy the canister, then run:

   $ pytest --network=[local/ic]
"""

# pylint: disable=missing-function-docstring, unused-import, wildcard-import, unused-wildcard-import

import re
import time
from pathlib import Path

import pytest

from icpp.smoketest import call_canister_api

DFX_JSON_PATH = Path(__file__).parent / "../dfx.json"
CANISTER_NAME = "my_canister"


def _parse_nat64(response: str) -> int:
    # Candid output looks like: "(123 : nat64)"  — possibly with underscores
    # in the integer literal for grouping.
    m = re.search(r"\(\s*([0-9_]+)\s*:\s*nat64\s*\)", response)
    if not m:
        pytest.fail(f"could not parse nat64 from response: {response!r}")
    return int(m.group(1).replace("_", ""))


def _reset(network: str) -> None:
    call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="reset_counts",
        network=network,
    )


def test__one_shot_fires(network: str, principal: str) -> None:
    _reset(network)

    # Schedule a one-shot timer (delay = 0). The IC will fire
    # canister_global_timer almost immediately; give it a moment.
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="start_one_shot",
        network=network,
    )
    timer_id = _parse_nat64(response)
    assert timer_id > 0

    # The IC schedules canister_global_timer asynchronously. Poll for up to
    # 10 seconds for the counter to advance.
    deadline = time.time() + 10
    count = 0
    while time.time() < deadline:
        response = call_canister_api(
            dfx_json_path=DFX_JSON_PATH,
            canister_name=CANISTER_NAME,
            canister_method="get_one_shot_count",
            network=network,
        )
        count = _parse_nat64(response)
        if count >= 1:
            break
        time.sleep(0.5)

    assert count == 1, f"one-shot did not fire (count={count})"


def test__recurring_fires_and_can_be_cancelled(network: str, principal: str) -> None:
    _reset(network)

    # Schedule a recurring timer with period 1s.
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="start_recurring",
        canister_argument="(1_000_000_000 : nat64)",
        network=network,
    )
    timer_id = _parse_nat64(response)
    assert timer_id > 0

    # Wait ~3.5 seconds; expect at least 3 firings.
    time.sleep(3.5)
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="get_recurring_count",
        network=network,
    )
    after_three = _parse_nat64(response)
    assert after_three >= 3, f"expected >=3 firings, got {after_three}"

    # Cancel and verify the count stops growing (allow at most one
    # additional in-flight firing tick).
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="stop_timer",
        canister_argument=f"({timer_id} : nat64)",
        network=network,
    )
    assert "true" in response

    time.sleep(2.5)
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="get_recurring_count",
        network=network,
    )
    after_cancel = _parse_nat64(response)
    assert after_cancel <= after_three + 1, (
        f"recurring did not stop after cancel: "
        f"after_three={after_three}, after_cancel={after_cancel}"
    )


def test__cancel_unknown_returns_false(network: str, principal: str) -> None:
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="stop_timer",
        canister_argument="(999_999 : nat64)",
        network=network,
    )
    assert "false" in response
