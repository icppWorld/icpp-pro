"""Test the lifecycle helpers of IC_Management on the local network:
update_settings, start, stop and delete a canister, deposit_cycles, and the
full canister_status. Each test works on a child that canister_management
creates, so controls.

$ pytest --network=local
"""

# pylint: disable=missing-function-docstring, unused-import, wildcard-import, unused-wildcard-import

from typing import Dict

import pytest

from icpp.smoketest import flatten_candid_text, get_canister_id

from .test_apis import CANISTER, ICP_YAML_PATH, call


def text(response: str) -> str:
    """The text of a '("...")' response"""
    return flatten_candid_text(response).strip('()", ')


def new_child(network: str) -> str:
    return call(network, "create_canister").split('"')[1]


def status(network: str, canister_id: str) -> Dict[str, str]:
    """The fields of the full canister_status, by name"""
    reply = text(call(network, "canister_status_full", f'(principal "{canister_id}")'))
    if reply.startswith(("rej:", "err:")):
        return {"error": reply}
    return dict(field.split("=", 1) for field in reply.split("; "))


def update_settings(network: str, canister_id: str, case: str) -> str:
    return text(
        call(network, "update_settings", f'(principal "{canister_id}", "{case}")')
    )


def lifecycle(network: str, method: str, canister_id: str) -> str:
    return text(call(network, method, f'(principal "{canister_id}")'))


def test__status_of_a_new_child(network: str) -> None:
    s = status(network, new_child(network))
    assert s["status"] == "running" and s["module_hash"] == "none", s
    assert s["controllers"] == "1", s
    assert int(s["cycles"]) > 0 and int(s["version"]) >= 0, s
    assert s["log_visibility"] == "controllers", s


def test__status_of_itself(network: str) -> None:
    me = get_canister_id(
        canister_name=CANISTER, icp_yaml_path=ICP_YAML_PATH, network=network
    )
    s = status(network, me)
    assert s["status"] == "running" and s["module_hash"] == "set", s
    assert int(s["version"]) > 0 and int(s["wasm_memory_size"]) > 0, s


def test__update_settings_controllers(network: str) -> None:
    child = new_child(network)
    assert update_settings(network, child, "controllers") == "ok"
    assert status(network, child)["controllers"] == "2"


def test__update_settings_log_visibility(network: str) -> None:
    child = new_child(network)
    assert update_settings(network, child, "public_logs") == "ok"
    assert status(network, child)["log_visibility"] == "public"


def test__update_settings_viewers(network: str, principal: str) -> None:
    child = new_child(network)
    assert update_settings(network, child, "viewers") == "ok"
    s = status(network, child)
    assert s["log_visibility"] == f"allowed_viewers({principal})", s
    assert s["status_visibility"] == f"allowed_viewers({principal})", s
    assert s["snapshot_visibility"] == "public", s


def test__update_settings_environment_variables(network: str) -> None:
    child = new_child(network)
    assert update_settings(network, child, "env") == "ok"
    assert status(network, child)["environment_variables"] == "A=1,B=two"


def test__update_settings_limits(network: str) -> None:
    child = new_child(network)
    assert update_settings(network, child, "limits") == "ok"
    s = status(network, child)
    assert s["freezing_threshold"] == "2592000", s
    assert s["reserved_cycles_limit"] == "5000000000000", s
    assert s["wasm_memory_limit"] == "3221225472", s
    assert s["wasm_memory_threshold"] == "1073741824", s
    assert s["log_memory_limit"] == "4096", s


def test__update_settings_nothing(network: str) -> None:
    # No field set: nothing changes
    child = new_child(network)
    before = status(network, child)
    assert update_settings(network, child, "none") == "ok"
    after = status(network, child)
    for field in ("controllers", "freezing_threshold", "log_visibility"):
        assert after[field] == before[field], (before, after)


def test__update_settings_not_a_controller(network: str) -> None:
    # canister_management does not control itself: the IC rejects the call
    me = get_canister_id(
        canister_name=CANISTER, icp_yaml_path=ICP_YAML_PATH, network=network
    )
    reply = update_settings(network, me, "none")
    assert reply.startswith("rej:"), reply


def test__stop_start_delete(network: str) -> None:
    child = new_child(network)
    assert lifecycle(network, "stop_child", child) == "ok"
    assert status(network, child)["status"] == "stopped"
    assert lifecycle(network, "start_child", child) == "ok"
    assert status(network, child)["status"] == "running"
    # A running canister cannot be deleted
    reply = lifecycle(network, "delete_child", child)
    assert reply.startswith("rej:") and "stopped" in reply, reply
    assert lifecycle(network, "stop_child", child) == "ok"
    assert lifecycle(network, "delete_child", child) == "ok"
    assert status(network, child)["error"].startswith("rej:")


def test__deposit_cycles(network: str) -> None:
    child = new_child(network)
    before = int(status(network, child)["cycles"])
    amount = 1_000_000_000
    reply = text(call(network, "deposit_child", f'(principal "{child}", {amount})'))
    assert reply == "ok", reply
    after = int(status(network, child)["cycles"])
    # The idle burn between the two reads is far below the amount
    assert before + amount * 0.99 <= after <= before + amount, (before, after)


def test__deposit_cycles_no_such_canister(network: str) -> None:
    reply = text(call(network, "deposit_child", '(principal "aaaaa-aa", 1_000_000)'))
    assert reply.startswith("rej:"), reply


def test__ecdsa_secp256r1(network: str) -> None:
    reply = text(
        call(network, "ecdsa_r1", '("test_key_1", blob "' + "\\07" * 32 + '")')
    )
    if reply.startswith("rej:") and "not found" in reply.lower():
        pytest.skip(f"no secp256r1 test_key_1 on this replica: {reply}")
    assert reply == "public_key=33 bytes; signature=64 bytes", reply
