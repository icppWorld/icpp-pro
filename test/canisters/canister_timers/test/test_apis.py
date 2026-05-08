"""Test canister APIs for canister_timers.

First deploy the canister, then run:

$ pytest --network=[local/ic]
"""

# pylint: disable=missing-function-docstring, unused-import, wildcard-import, unused-wildcard-import

import re
import time
from pathlib import Path
from typing import Callable

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


def _parse_bool(response: str) -> bool:
    # Candid `(bool)` reply renders as exactly "(true)" or "(false)" (with
    # optional surrounding whitespace). A substring check on "true" / "false"
    # would also match unrelated text in error responses, so anchor the match.
    m = re.search(r"\(\s*(true|false)\s*\)", response)
    if not m:
        pytest.fail(f"could not parse bool from response: {response!r}")
    return m.group(1) == "true"


def _poll_count(
    network: str,
    method: str,
    predicate: Callable[[int], bool],
    timeout_s: float = 10.0,
    interval_s: float = 0.2,
) -> int:
    """Repeatedly query a `() -> (nat64) query` method until `predicate(count)`
    returns True or the deadline is reached. Returns the last observed count.

    Avoids fixed sleeps that have to be tuned to the slowest realistic
    replica. CI pytest runs on shared infrastructure where a 3 s sleep can
    be both flaky-too-short on a busy box and wasteful on a quiet one;
    polling adapts."""
    deadline = time.time() + timeout_s
    count = 0
    while time.time() < deadline:
        count = _parse_nat64(
            call_canister_api(
                dfx_json_path=DFX_JSON_PATH,
                canister_name=CANISTER_NAME,
                canister_method=method,
                network=network,
            )
        )
        if predicate(count):
            return count
        time.sleep(interval_s)
    return count


def _reset(network: str) -> None:
    call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="reset_counts",
        network=network,
    )


def test__one_shot_fires(network: str, principal: str) -> None:
    _reset(network)

    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="start_one_shot",
        network=network,
    )
    timer_id = _parse_nat64(response)
    assert timer_id > 0

    count = _poll_count(network, "get_one_shot_count", lambda c: c >= 1)
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

    # Poll until we observe at least 3 firings (or hit the deadline).
    after_three = _poll_count(
        network, "get_recurring_count", lambda c: c >= 3, timeout_s=15.0
    )
    assert after_three >= 3, f"expected >=3 firings, got {after_three}"

    # Cancel.
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="stop_timer",
        canister_argument=f"({timer_id} : nat64)",
        network=network,
    )
    assert _parse_bool(response) is True

    # Quietness check still requires a wall-clock wait — there's no faster
    # way to be sure something *isn't* happening — but we keep it short and
    # allow at most one additional in-flight firing tick that may have been
    # in the pipeline when cancel landed.
    time.sleep(2.5)
    after_cancel = _parse_nat64(
        call_canister_api(
            dfx_json_path=DFX_JSON_PATH,
            canister_name=CANISTER_NAME,
            canister_method="get_recurring_count",
            network=network,
        )
    )
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
    assert _parse_bool(response) is False


def test__reset_counts_cancels_recurring_timer(network: str, principal: str) -> None:
    """Leak-proof reset (commit 4): reset_counts now cancels every
    registered timer in addition to zeroing counters. After reset, a
    previously-registered recurring timer must NOT continue to fire."""
    _reset(network)

    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="start_recurring",
        canister_argument="(1_000_000_000 : nat64)",
        network=network,
    )
    timer_id = _parse_nat64(response)
    assert timer_id > 0

    initial_count = _poll_count(
        network, "get_recurring_count", lambda c: c >= 2, timeout_s=10.0
    )
    assert initial_count >= 2, (
        f"recurring timer was not firing; expected >=2 ticks within poll window, "
        f"got {initial_count}"
    )

    # reset_counts now also calls IC_API::cancel_all_timers().
    call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="reset_counts",
        network=network,
    )

    # Quietness check: wait long enough that, if the timer leaked, the
    # count would advance. Fixed sleep is the right shape here — we're
    # asserting non-occurrence, not waiting for an event.
    time.sleep(3.0)
    after_reset_count = _parse_nat64(
        call_canister_api(
            dfx_json_path=DFX_JSON_PATH,
            canister_name=CANISTER_NAME,
            canister_method="get_recurring_count",
            network=network,
        )
    )
    assert after_reset_count == 0, (
        f"recurring timer leaked across reset: count grew to "
        f"{after_reset_count} after reset_counts"
    )
