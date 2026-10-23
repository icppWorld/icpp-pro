"""Test the observability helpers of IC_Management on the local network:
fetch_canister_logs (with filters and log_visibility), canister_metrics,
subnet_info, subnet_metrics and node_metrics_history.

$ pytest --network=local
"""

# pylint: disable=missing-function-docstring, unused-import, wildcard-import, unused-wildcard-import

from typing import List

import pytest

from .test_apis import call
from .test_code import install
from .test_lifecycle import new_child, text, update_settings


def logs(
    network: str, child: str, kind: str = "none", start: int = 0, end: int = 0
) -> List[str]:
    reply = text(
        call(
            network,
            "logs_child",
            f'(principal "{child}", "{kind}", {start} : nat64, {end} : nat64)',
        )
    )
    if reply.startswith(("rej:", "err:")):
        return [reply]
    # '<idx>:<content in hex>|...' -> ['<idx>:<content>', ...]
    records = []
    for record in reply.split("|") if reply else []:
        idx, content = record.split(":", 1)
        records.append(f"{idx}:{bytes.fromhex(content).decode(errors='replace')}")
    return records


def skip_if_unsupported(reply: str) -> None:
    if reply.startswith("rej:") and (
        "not supported" in reply.lower()
        or "unknown" in reply.lower()
        or "not found" in reply.lower()
        or "not available" in reply.lower()
    ):
        pytest.skip(f"not supported by this replica: {reply}")


def test__logs(network: str) -> None:
    child = new_child(network)
    assert install(network, child, "install") == "ok"
    assert install(network, child, "upgrade") == "ok"
    records = logs(network, child)
    skip_if_unsupported(records[0] if records else "")
    # canister_init and canister_post_upgrade each printed "hi"
    contents = [r.split(":", 1)[1] for r in records]
    assert contents[-2:] == ["hi", "hi"], records
    first_idx = int(records[-2].split(":")[0])
    by_idx = logs(network, child, "by_idx", first_idx + 1, first_idx + 2)
    assert by_idx == [records[-1]], (records, by_idx)
    everything = logs(network, child, "by_timestamp_nanos", 0, 2**64 - 1)
    assert everything[-2:] == records[-2:], everything


def test__logs_visibility(network: str) -> None:
    child = new_child(network)
    assert install(network, child, "install") == "ok"
    # Not a controller and not public: rejected
    assert update_settings(network, child, "only_caller") == "ok"
    reply = logs(network, child)[0]
    assert reply.startswith("rej:"), reply

    public = new_child(network)
    assert install(network, public, "install") == "ok"
    assert update_settings(network, public, "public_logs") == "ok"
    assert update_settings(network, public, "only_caller") == "ok"
    records = logs(network, public)
    assert records and records[-1].endswith(":hi"), records


def test__canister_metrics(network: str) -> None:
    child = new_child(network)
    assert install(network, child, "install") == "ok"
    reply = text(call(network, "metrics_child", f'(principal "{child}")'))
    skip_if_unsupported(reply)
    assert reply.startswith("memory="), reply
    fields = dict(f.split("=") for f in reply.split("; "))
    assert (
        int(fields["instructions"]) >= 0 and int(fields["canister_creation"]) >= 0
    ), fields


def test__subnet_info(network: str) -> None:
    # "aaaaa-aa": the subnet of canister_management itself
    reply = text(call(network, "subnet_info_of", '(principal "aaaaa-aa")'))
    skip_if_unsupported(reply)
    assert (
        reply.startswith("replica_version=") and "replica_version=;" not in reply
    ), reply


def test__subnet_metrics(network: str) -> None:
    reply = text(call(network, "subnet_metrics_of", '(principal "aaaaa-aa")'))
    skip_if_unsupported(reply)
    fields = dict(f.split("=") for f in reply.split("; "))
    assert int(fields["num_canisters"]) >= 1, fields


def test__node_metrics_history(network: str) -> None:
    reply = text(call(network, "node_metrics_of", '(principal "aaaaa-aa", 0 : nat64)'))
    skip_if_unsupported(reply)
    assert not reply.startswith(("rej:", "err:")), reply
    for entry in reply.split("|") if reply else []:
        timestamp, nodes = entry.split(":", 1)
        assert int(timestamp) >= 0
        for node in nodes.split(",") if nodes else []:
            assert len(node.split("/")) == 3, node
