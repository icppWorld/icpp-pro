"""Test that a user-supplied strong canister_global_timer overrides the
   icpp-pro weak built-in dispatcher end-to-end on a real local replica.

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
    m = re.search(r"\(\s*([0-9_]+)\s*:\s*nat64\s*\)", response)
    if not m:
        pytest.fail(f"could not parse nat64 from response: {response!r}")
    return int(m.group(1).replace("_", ""))


def test__user_override_wins_on_real_ic(network: str, principal: str) -> None:
    # Register a 1 s recurring IcTimers timer via the user's update method.
    # Capture the id so we can cancel it in the finally block — keeps the
    # suite re-runnable even if an assertion below fails.
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="start_user_timer",
        canister_argument="(1_000_000_000 : nat64)",
        network=network,
    )
    timer_id = _parse_nat64(response)
    assert timer_id > 0

    try:
        # Poll until the user override has fired at least once, with a
        # bounded deadline. This adapts to replica load — fixed sleeps were
        # both flake-prone on busy boxes (too short) and wasteful on quiet
        # ones (too long).
        deadline = time.time() + 15.0
        user_calls = 0
        while time.time() < deadline:
            user_calls = _parse_nat64(
                call_canister_api(
                    dfx_json_path=DFX_JSON_PATH,
                    canister_name=CANISTER_NAME,
                    canister_method="get_user_dispatcher_calls",
                    network=network,
                )
            )
            if user_calls >= 1:
                break
            time.sleep(0.2)

        # Now query the other two counters to confirm side-effect absence.
        icpp_calls = _parse_nat64(
            call_canister_api(
                dfx_json_path=DFX_JSON_PATH,
                canister_name=CANISTER_NAME,
                canister_method="get_icpp_callback_calls",
                network=network,
            )
        )
        icpp_timer_count = _parse_nat64(
            call_canister_api(
                dfx_json_path=DFX_JSON_PATH,
                canister_name=CANISTER_NAME,
                canister_method="get_icpp_timer_count",
                network=network,
            )
        )

        # Range-only assertions — exact recurring-fire counts will flake.
        assert (
            user_calls >= 1
        ), f"user override did not run on the IC (user_calls={user_calls})"
        assert (
            icpp_calls == 0
        ), f"icpp-pro built-in dispatcher fired despite user override (icpp_calls={icpp_calls})"
        assert (
            icpp_timer_count >= 1
        ), f"icpp-pro registry was drained — dispatcher must have run (icpp_timer_count={icpp_timer_count})"
    finally:
        # Always cancel via the rev0 IC_API::cancel_timer(id) public API
        # so the registry is empty and the suite is re-runnable.
        call_canister_api(
            dfx_json_path=DFX_JSON_PATH,
            canister_name=CANISTER_NAME,
            canister_method="stop_user_timer",
            canister_argument=f"({timer_id} : nat64)",
            network=network,
        )
