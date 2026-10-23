"""Test the outcall and provisional helpers of IC_Management on the local
network: flexible_http_request, http_request with pay-as-you-go pricing, and
provisional_create_canister_with_cycles and provisional_top_up_canister.

$ pytest --network=local
"""

# pylint: disable=missing-function-docstring, unused-import, wildcard-import, unused-wildcard-import

import pytest

from .test_apis import call
from .test_lifecycle import status, text

URL = "https://example.com/"


def flexible(
    network: str,
    min_responses: int,
    max_responses: int,
    total: int,
    with_replication: bool = True,
) -> str:
    return text(
        call(
            network,
            "flexible_get",
            f'("{URL}", {min_responses} : nat32, {max_responses} : nat32, {total} : nat32, '
            f"{str(with_replication).lower()})",
        )
    )


def skip_without_internet(reply: str) -> None:
    if reply.startswith("rej:") and (
        "connect" in reply.lower()
        or "dns" in reply.lower()
        or "timed out" in reply.lower()
    ):
        pytest.skip(f"the HTTPS outcall did not reach the internet: {reply}")


def skip_on_server_error(status: str) -> None:
    # The outcall worked: the server had a problem (seen on CI: 503)
    if status.isdigit() and int(status) >= 500:
        pytest.skip(f"example.com answered {status}")


def test__flexible_http_request(network: str) -> None:
    reply = flexible(network, 1, 1, 1)
    skip_without_internet(reply)
    assert reply.startswith("ok:"), reply
    statuses = reply[len("ok:") :].split(",")
    skip_on_server_error(statuses[0])
    assert len(statuses) == 1 and statuses[0] == "200", reply


def test__flexible_http_request_default_replication(network: str) -> None:
    reply = flexible(network, 0, 0, 0, with_replication=False)
    skip_without_internet(reply)
    assert reply.startswith(("ok:", "err:")), reply


def test__flexible_http_request_impossible_replication(network: str) -> None:
    # min_responses larger than max_responses: rejected
    reply = flexible(network, 3, 2, 2)
    assert reply.startswith("rej:"), reply


def test__http_request_pay_as_you_go(network: str) -> None:
    for replicated in ("true", "false"):
        reply = text(call(network, "http_get_v2", f'("{URL}", {replicated})'))
        skip_without_internet(reply)
        skip_on_server_error(reply.split(";")[0].removeprefix("status="))
        assert reply.startswith("status=200; bytes="), reply
        assert int(reply.split("bytes=")[1]) > 0, reply


def test__provisional_create_and_top_up(network: str) -> None:
    child = text(
        call(network, "provisional_create", "(vec { 3_000_000_000_000 : nat }, vec {})")
    )
    assert not child.startswith(("rej:", "err:")), child
    before = int(status(network, child)["cycles"])
    assert before > 2_000_000_000_000, before
    amount = 1_000_000_000_000
    reply = text(
        call(network, "provisional_top_up", f'(principal "{child}", {amount})')
    )
    assert reply == "ok", reply
    after = int(status(network, child)["cycles"])
    assert before + amount * 0.99 <= after <= before + amount, (before, after)


def test__provisional_create_default_amount(network: str) -> None:
    child = text(call(network, "provisional_create", "(vec {}, vec {})"))
    assert not child.startswith(("rej:", "err:")), child
    assert int(status(network, child)["cycles"]) > 0
