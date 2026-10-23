"""Test the code helpers of IC_Management on the local network: install_code
in every mode, the chunk store, install_chunked_code, uninstall_code,
canister_metadata and canister_info. Each test works on a child that
canister_management creates, so controls.

$ pytest --network=local
"""

# pylint: disable=missing-function-docstring, unused-import, wildcard-import, unused-wildcard-import

import gzip
import hashlib
from typing import List

from icpp.smoketest import get_canister_id

from .test_apis import CANISTER, ICP_YAML_PATH, blob_arg, call
from .test_lifecycle import new_child, text, update_settings
from .wasm_module import wasm_module

WASM = wasm_module()


def install(network: str, child: str, mode: str, wasm: bytes = WASM) -> str:
    return text(
        call(
            network,
            "install_child",
            f'(principal "{child}", "{mode}", {blob_arg(wasm)}, blob "")',
        )
    )


def info(network: str, child: str, count: str = "opt (20 : nat64)") -> str:
    return text(call(network, "info_child", f'(principal "{child}", {count})'))


def changes(network: str, child: str) -> List[str]:
    return info(network, child).split("changes=")[1].split(" ")


def metadata(network: str, child: str, name: str) -> str:
    return text(call(network, "metadata_child", f'(principal "{child}", "{name}")'))


def test__install_and_info(network: str) -> None:
    child = new_child(network)
    assert install(network, child, "install") == "ok"
    reply = info(network, child)
    assert f"module_hash={hashlib.sha256(WASM).hexdigest()};" in reply, reply
    assert "controllers=1;" in reply, reply
    c = changes(network, child)
    # Made by canister_management, with its canister version
    assert c[0].startswith("creation(1") and "/canister@" in c[0], c
    assert c[1].startswith("code_deployment(install,"), c
    assert "/canister@" in c[1], c


def test__upgrade_and_reinstall_modes(network: str) -> None:
    child = new_child(network)
    assert install(network, child, "install") == "ok"
    assert install(network, child, "upgrade") == "ok"
    assert install(network, child, "upgrade_skip") == "ok"
    assert install(network, child, "reinstall") == "ok"
    modes = [c.split("(")[1].split(",")[0] for c in changes(network, child)[1:]]
    assert modes == ["install", "upgrade", "upgrade", "reinstall"], modes


def test__upgrade_keep_wasm_memory(network: str) -> None:
    # keep: only for a canister with enhanced orthogonal persistence; a
    # wasm32 module without it is rejected, which shows the option arrived
    child = new_child(network)
    assert install(network, child, "install") == "ok"
    reply = install(network, child, "upgrade_keep")
    assert reply == "ok" or (reply.startswith("rej:") and "persist" in reply), reply


def test__install_gzip(network: str) -> None:
    child = new_child(network)
    assert install(network, child, "install", gzip.compress(WASM)) == "ok"
    assert "code_deployment(install," in info(network, child)


def test__install_invalid_wasm(network: str) -> None:
    reply = install(network, new_child(network), "install", b"not a wasm")
    assert reply.startswith("rej:"), reply


def test__install_not_a_controller(network: str) -> None:
    child = new_child(network)
    assert update_settings(network, child, "only_caller") == "ok"
    reply = install(network, child, "install")
    assert reply.startswith("rej:"), reply


def test__metadata(network: str) -> None:
    child = new_child(network)
    assert install(network, child, "install") == "ok"
    assert metadata(network, child, "icpp") == "value=public-data"
    # icp:private: readable by a controller
    assert metadata(network, child, "icpp_secret") == "value=secret"
    reply = metadata(network, child, "no_such_section")
    assert reply.startswith("rej:"), reply


def test__metadata_not_a_controller(network: str) -> None:
    child = new_child(network)
    assert install(network, child, "install") == "ok"
    assert update_settings(network, child, "only_caller") == "ok"
    # icp:public: anyone; icp:private: controllers only
    assert metadata(network, child, "icpp") == "value=public-data"
    reply = metadata(network, child, "icpp_secret")
    assert reply.startswith("rej:"), reply


def test__metadata_of_itself(network: str) -> None:
    me = get_canister_id(
        canister_name=CANISTER, icp_yaml_path=ICP_YAML_PATH, network=network
    )
    assert metadata(network, me, "candid:service").startswith("value=")


def test__chunked_install(network: str) -> None:
    child = new_child(network)
    chunks = [WASM[:60], WASM[60:120], WASM[120:]]
    hashes = []
    for chunk in chunks:
        reply = text(
            call(
                network,
                "upload_chunk_child",
                f'(principal "{child}", {blob_arg(chunk)})',
            )
        )
        assert reply == hashlib.sha256(chunk).hexdigest(), reply
        hashes.append(reply)
    # Uploading a chunk again does not add it twice
    text(
        call(
            network,
            "upload_chunk_child",
            f'(principal "{child}", {blob_arg(chunks[0])})',
        )
    )
    stored = text(call(network, "stored_chunks_child", f'(principal "{child}")'))
    assert sorted(stored.split(",")) == sorted(hashes), stored

    hash_args = "vec { " + "; ".join(blob_arg(bytes.fromhex(h)) for h in hashes) + " }"
    wrong = text(
        call(
            network,
            "install_chunked_child",
            f'(principal "{child}", "install", {hash_args}, {blob_arg(b"0" * 32)}, null)',
        )
    )
    assert wrong.startswith("rej:"), wrong
    module_hash = blob_arg(hashlib.sha256(WASM).digest())
    reply = text(
        call(
            network,
            "install_chunked_child",
            f'(principal "{child}", "install", {hash_args}, {module_hash}, null)',
        )
    )
    assert reply == "ok", reply
    assert f"module_hash={hashlib.sha256(WASM).hexdigest()};" in info(network, child)

    assert text(call(network, "clear_chunks_child", f'(principal "{child}")')) == "ok"
    assert text(call(network, "stored_chunks_child", f'(principal "{child}")')) == ""


def test__upload_chunk_too_large(network: str) -> None:
    child = new_child(network)
    ok = text(
        call(network, "upload_big_chunk_child", f'(principal "{child}", 1_048_576)')
    )
    assert ok == hashlib.sha256(b"x" * 1024 * 1024).hexdigest(), ok
    reply = text(
        call(network, "upload_big_chunk_child", f'(principal "{child}", 1_048_577)')
    )
    assert reply.startswith("rej:"), reply


def test__uninstall(network: str) -> None:
    child = new_child(network)
    assert install(network, child, "install") == "ok"
    assert text(call(network, "uninstall_child", f'(principal "{child}")')) == "ok"
    reply = info(network, child)
    assert "module_hash=none;" in reply, reply
    assert changes(network, child)[-1].startswith("code_uninstall"), reply


def test__info_num_requested_changes(network: str) -> None:
    child = new_child(network)
    assert install(network, child, "install") == "ok"
    assert install(network, child, "upgrade") == "ok"
    reply = info(network, child, "opt (1 : nat64)")
    assert reply.startswith("total=3;"), reply
    assert len(reply.split("changes=")[1].split(" ")) == 1, reply
    assert "changes=code_deployment(upgrade," in reply, reply


def test__info_settings_change(network: str) -> None:
    # A change of the controllers is in the history: a controllers_change,
    # or a settings_change on a newer replica
    child = new_child(network)
    assert update_settings(network, child, "controllers") == "ok"
    last = changes(network, child)[-1]
    assert last.startswith(("controllers_change(2)", "settings_change(2")), last
