"""Test the management canister helpers (ic_management.h)

Each method makes a priced call for real on the local network, attaching the
cycles of its cost_* function: the call succeeding shows they were enough.
First deploy, then run:

$ pytest --network=local
"""

# pylint: disable=missing-function-docstring, unused-import, wildcard-import, unused-wildcard-import

import hashlib
from pathlib import Path
from typing import List

import pytest
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives.asymmetric.utils import (
    Prehashed,
    encode_dss_signature,
)

from icpp.smoketest import call_canister_api, flatten_candid_text

# Path to the icp.yaml file
ICP_YAML_PATH = Path(__file__).parent / "../icp.yaml"
CANISTER = "canister_management"
KEY = "test_key_1"


def call(
    network: str, method: str, argument: str = "()", timeout_seconds: int = 120
) -> str:
    return call_canister_api(
        icp_yaml_path=ICP_YAML_PATH,
        canister_name=CANISTER,
        canister_method=method,
        canister_argument=argument,
        network=network,
        timeout_seconds=timeout_seconds,
    )


def blobs(response: str) -> List[bytes]:
    """The blobs of a Candid text response, in order. A blob is written as
    printable characters and \\XX escapes (two hex digits)."""
    out = []
    for part in response.split('blob "')[1:]:
        data = bytearray()
        i = 0
        while part[i] != '"':
            if part[i] == "\\":
                if part[i + 1] in "\\\"'":
                    data.append(ord(part[i + 1]))
                    i += 2
                else:
                    data.append(int(part[i + 1 : i + 3], 16))
                    i += 3
            else:
                data.append(ord(part[i]))
                i += 1
        out.append(bytes(data))
    return out


def blob_arg(data: bytes) -> str:
    return 'blob "' + "".join(f"\\{b:02x}" for b in data) + '"'


def test__raw_rand(network: str) -> None:
    a = blobs(call(network, "raw_rand"))[0]
    b = blobs(call(network, "raw_rand"))[0]
    assert len(a) == 32 and len(b) == 32 and a != b


def test__create_canister(network: str) -> None:
    # Created with exactly cost_create_canister() attached
    response = call(network, "create_canister")
    assert response.startswith('(principal "') and response.endswith('")'), response


def test__ecdsa(network: str) -> None:
    # A real threshold ECDSA signature, verified with the public key
    message_hash = hashlib.sha256(b"icpp-pro").digest()
    public_key, signature = blobs(
        call(network, "ecdsa", f'("{KEY}", {blob_arg(message_hash)})')
    )
    assert len(public_key) == 33 and len(signature) == 64
    # The signature is r || s; verify() raises InvalidSignature if it is wrong
    vk = ec.EllipticCurvePublicKey.from_encoded_point(ec.SECP256K1(), public_key)
    r, s = (int.from_bytes(signature[i : i + 32], "big") for i in (0, 32))
    vk.verify(
        encode_dss_signature(r, s), message_hash, ec.ECDSA(Prehashed(hashes.SHA256()))
    )


@pytest.mark.parametrize("ed25519", [True, False])
def test__schnorr(network: str, ed25519: bool) -> None:
    public_key, signature = blobs(
        call(
            network,
            "schnorr",
            f'("{KEY}", {str(ed25519).lower()}, {blob_arg(b"icpp-pro")})',
        )
    )
    assert len(public_key) == (32 if ed25519 else 33)
    assert len(signature) == 64


def test__vetkd(network: str) -> None:
    public_key, encrypted_key = blobs(call(network, "vetkd", f'("{KEY}")'))
    assert len(public_key) == 96  # a G2 point
    assert len(encrypted_key) == 192  # G1 + G2 + G1


def test__invalid_key(network: str) -> None:
    # The cost of an unknown key can't be computed: the call is not made
    assert call(network, "invalid_key") == '("err:unknown key name")'


@pytest.mark.parametrize("replicated", [True, False])
def test__http_get(network: str, replicated: bool) -> None:
    # A real HTTPS outcall to the internet: replicated through the transform,
    # which removes the headers, or non-replicated
    response = call(
        network, "http_get", f'("https://example.com", {str(replicated).lower()})'
    )
    text = flatten_candid_text(response).strip('()" ')
    if text.startswith("rej:"):
        pytest.skip(f"the HTTPS outcall did not reach the internet: {text}")
    status, body_size, headers = (int(x) for x in text.split())
    assert status == 200 and body_size > 0
    if replicated:
        assert headers == 0
    else:
        assert headers > 0


def test__cost_http_v2(network: str) -> None:
    # Pay-as-you-go pricing: a non-replicated outcall (one node) is cheaper
    # than a fully replicated one, and than one on 4 nodes. (The local
    # subnet is small: 4 nodes can cost more than all of its nodes.)
    response = flatten_candid_text(call(network, "cost_http_v2"))
    fully, non_replicated, flexible = (
        int(x.split(":")[0].strip().replace("_", ""))
        for x in response.strip("()").split(",")
        if ":" in x
    )
    assert fully > non_replicated > 0
    assert flexible > non_replicated


def test__echo_vec_blob(network: str) -> None:
    # icpp-candid's vec blob, decoded and encoded
    response = call(
        network, "echo_vec_blob", '(vec { blob "\\01\\02"; blob ""; blob "abc" })'
    )
    assert blobs(response) == [b"\x01\x02", b"", b"abc"]
