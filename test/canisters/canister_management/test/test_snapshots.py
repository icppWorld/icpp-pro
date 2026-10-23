"""Test the snapshot helpers of IC_Management on the local network: take,
list, load and delete a snapshot, read its metadata and data, and copy it by
uploading what was read. Each test works on a child that canister_management
creates, so controls.

$ pytest --network=local
"""

# pylint: disable=missing-function-docstring, unused-import, wildcard-import, unused-wildcard-import

import hashlib
from typing import Dict, List

from .test_apis import blob_arg, call
from .test_code import WASM, changes, install, info
from .test_lifecycle import lifecycle, new_child, status, text


def stopped_child_with_code(network: str) -> str:
    child = new_child(network)
    assert install(network, child, "install") == "ok"
    assert lifecycle(network, "stop_child", child) == "ok"
    return child


def take(
    network: str, child: str, replace: str = "vec {}", uninstall: str = "null"
) -> str:
    return text(
        call(
            network,
            "take_snapshot_child",
            f'(principal "{child}", {replace}, {uninstall})',
        )
    )


def snapshot_ids(network: str, child: str) -> List[str]:
    listed = text(call(network, "list_snapshots_child", f'(principal "{child}")'))
    return [s.split(":")[0] for s in listed.split(",")] if listed else []


def metadata(network: str, child: str, snapshot_id: str) -> Dict[str, str]:
    reply = text(
        call(
            network,
            "snapshot_metadata_child",
            f'(principal "{child}", {blob_arg(bytes.fromhex(snapshot_id))})',
        )
    )
    assert not reply.startswith(("rej:", "err:")), reply
    return dict(field.split("=", 1) for field in reply.split("; "))


def read(
    network: str, child: str, snapshot_id: str, kind: str, offset: int, size: int
) -> str:
    return text(
        call(
            network,
            "read_snapshot_data_child",
            f'(principal "{child}", {blob_arg(bytes.fromhex(snapshot_id))}, "{kind}", '
            f'{offset} : nat64, {size} : nat64, blob "")',
        )
    )


def snapshot_call(network: str, method: str, child: str, snapshot_id: str) -> str:
    return text(
        call(
            network,
            method,
            f'(principal "{child}", {blob_arg(bytes.fromhex(snapshot_id))})',
        )
    )


def test__take_list_metadata(network: str) -> None:
    child = stopped_child_with_code(network)
    taken = take(network, child)
    snapshot_id, size = taken.split(":")
    assert int(size) > 0, taken
    assert snapshot_ids(network, child) == [snapshot_id]
    md = metadata(network, child, snapshot_id)
    assert md["source"] == "taken_from_canister", md
    assert int(md["wasm_module_size"]) == len(WASM), md
    assert md["wasm_memory_size"] == str(64 * 1024), md
    assert md["stable_memory_size"] == "0", md


def test__read_data(network: str) -> None:
    child = stopped_child_with_code(network)
    snapshot_id = take(network, child).split(":")[0]
    # The module, in two parts, is the installed module
    first = read(network, child, snapshot_id, "wasm_module", 0, 100)
    rest = read(network, child, snapshot_id, "wasm_module", 100, len(WASM) - 100)
    assert bytes.fromhex(first + rest) == WASM
    # The wasm memory starts with the module's data segment, "hi"
    assert read(network, child, snapshot_id, "wasm_memory", 0, 2) == "hi".encode().hex()
    # Past the end: rejected
    reply = read(network, child, snapshot_id, "wasm_module", len(WASM), 1)
    assert reply.startswith("rej:"), reply


def test__copy_load_delete(network: str) -> None:
    child = stopped_child_with_code(network)
    # A chunk in its chunk store, so the copy has one to copy too
    chunk = WASM[:60]
    chunk_hash = text(
        call(network, "upload_chunk_child", f'(principal "{child}", {blob_arg(chunk)})')
    )
    assert chunk_hash == hashlib.sha256(chunk).hexdigest(), chunk_hash
    original = take(network, child).split(":")[0]
    assert metadata(network, child, original)["chunks"] == "1"
    copy = snapshot_call(network, "copy_snapshot_child", child, original)
    assert copy != original and not copy.startswith(("rej:", "err:")), copy
    assert sorted(snapshot_ids(network, child)) == sorted([original, copy])
    md = metadata(network, child, copy)
    assert md["source"] == "metadata_upload", md
    assert int(md["wasm_module_size"]) == len(WASM), md
    assert md["chunks"] == "1", md
    # The copy has the same module, and the same chunk
    copied = read(network, child, copy, "wasm_module", 0, len(WASM))
    assert bytes.fromhex(copied) == WASM

    # Loading the copy restores the chunk store
    assert text(call(network, "clear_chunks_child", f'(principal "{child}")')) == "ok"
    assert text(call(network, "stored_chunks_child", f'(principal "{child}")')) == ""
    assert snapshot_call(network, "load_snapshot_child", child, copy) == "ok"
    stored = text(call(network, "stored_chunks_child", f'(principal "{child}")'))
    assert stored == chunk_hash, stored
    assert changes(network, child)[-1].startswith(f"load_snapshot({copy},"), info(
        network, child
    )
    assert lifecycle(network, "start_child", child) == "ok"
    assert status(network, child)["status"] == "running"

    assert snapshot_call(network, "delete_snapshot_child", child, original) == "ok"
    assert snapshot_call(network, "delete_snapshot_child", child, copy) == "ok"
    assert snapshot_ids(network, child) == []


def test__replace_and_uninstall(network: str) -> None:
    child = stopped_child_with_code(network)
    first = take(network, child).split(":")[0]
    second = take(
        network, child, f"vec {{ {blob_arg(bytes.fromhex(first))} }}", "opt true"
    )
    second_id = second.split(":")[0]
    assert second_id != first, second
    assert snapshot_ids(network, child) == [second_id]
    # The code was uninstalled once the snapshot was taken
    assert "module_hash=none;" in info(network, child)


def test__delete_twice(network: str) -> None:
    child = stopped_child_with_code(network)
    snapshot_id = take(network, child).split(":")[0]
    assert snapshot_call(network, "delete_snapshot_child", child, snapshot_id) == "ok"
    reply = snapshot_call(network, "delete_snapshot_child", child, snapshot_id)
    assert reply.startswith("rej:"), reply


def test__no_such_snapshot(network: str) -> None:
    child = stopped_child_with_code(network)
    reply = text(
        call(
            network,
            "snapshot_metadata_child",
            f'(principal "{child}", {blob_arg(b"no such snapshot")})',
        )
    )
    assert reply.startswith("rej:"), reply
