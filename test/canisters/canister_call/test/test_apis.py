"""Test canister APIs for canister_call — exercises the IC_Call inter-canister
call helper.

The canister calls itself: `ping_self("hi")` schedules an inter-canister call
to its own `echo(text)` method and finishes the original message from inside
the on_reply callback. The reply payload is stashed in g_last_echoed which a
subsequent query verifies.

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


def _parse_text(response: str) -> str:
    m = re.search(r'\(\s*"([^"]*)"\s*\)', response)
    if not m:
        pytest.fail(f"could not parse text from response: {response!r}")
    return m.group(1)


def _parse_nat32(response: str) -> int:
    m = re.search(r"\(\s*([0-9_]+)\s*:\s*nat32\s*\)", response)
    if not m:
        pytest.fail(f"could not parse nat32 from response: {response!r}")
    return int(m.group(1).replace("_", ""))


def test_echo_directly(identity_anonymous: dict[str, str], network: str) -> None:
    """Sanity: the echo update returns its argument unchanged."""
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="echo",
        canister_argument='("hello")',
        network=network,
    )
    assert _parse_text(response) == "hello"


def test_ping_self_round_trip(
    identity_anonymous: dict[str, str], network: str
) -> None:
    """ping_self issues an inter-canister call to self::echo. After it
    returns, get_last_echoed should hold the echoed value."""
    # Fire the self-call.
    call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="ping_self",
        canister_argument='("ic_call_works")',
        network=network,
    )

    # Replica processes the reply almost immediately, but give it a generous
    # poll window for slow CI nodes.
    deadline = time.time() + 10.0
    last = ""
    while time.time() < deadline:
        response = call_canister_api(
            dfx_json_path=DFX_JSON_PATH,
            canister_name=CANISTER_NAME,
            canister_method="get_last_echoed",
            canister_argument="()",
            network=network,
        )
        last = _parse_text(response)
        if last == "ic_call_works":
            break
        time.sleep(0.2)

    assert last == "ic_call_works", (
        "expected echoed text after ping_self; "
        f"got {last!r} (g_pending_count was never observed dropping)"
    )


def test_pending_count_zero_after_settle(
    identity_anonymous: dict[str, str], network: str
) -> None:
    """After ping_self has been observed completing, the IC_Call pending
    registry should be empty (no leaked env cookies)."""
    response = call_canister_api(
        dfx_json_path=DFX_JSON_PATH,
        canister_name=CANISTER_NAME,
        canister_method="get_pending_count",
        canister_argument="()",
        network=network,
    )
    assert _parse_nat32(response) == 0
