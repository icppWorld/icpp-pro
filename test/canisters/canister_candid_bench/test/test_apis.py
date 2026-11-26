"""Candid performance benchmarks: instructions per decode and per encode

First deploy the canister, then run:

$ pytest --network=local

Each case checks that the canister decoded the right values, or encoded the
same bytes as didc, and compares its instructions with test/baseline.json,
within TOLERANCE (10%):
- more than 10% above the baseline fails;
- more than 10% below it passes, with a reminder to update the baseline.

To record new baselines, after a change that makes a case faster:

$ ICPP_CANDID_BENCH_UPDATE=1 pytest --network=local

An update still fails a case that got more than 10% slower, and does not
record it. To record a slowdown on purpose, use ICPP_CANDID_BENCH_UPDATE=force.
An update also drops the baseline of a case that no longer exists.

The results of the last run are in build/bench-results.json.
"""

# pylint: disable=missing-function-docstring, unused-import, wildcard-import, unused-wildcard-import

import json
import os
import re
import subprocess
import warnings
from pathlib import Path
from typing import Callable, Dict, Iterator, List, Tuple

import pytest
from icpp.smoketest import call_canister_api

# Path to the icp.yaml file
ICP_YAML_PATH = Path(__file__).parent / "../icp.yaml"

# Canister in the icp.yaml file we want to test
CANISTER_NAME = "my_canister"

DID_PATH = Path(__file__).parent / "../src/my_canister.did"
BASELINE_PATH = Path(__file__).parent / "baseline.json"
RESULTS_PATH = Path(__file__).parent / "../build/bench-results.json"

# A case may use this much more than its baseline before it fails, or this
# much less before the test reminds to update the baseline
TOLERANCE = 0.10

# ----------------------------------------------------------------------------
# The values, as src/my_canister.cpp builds them

MASK64 = (1 << 64) - 1


class Fnv1a:
    """The 64-bit FNV-1a hash and the number of bytes hashed"""

    def __init__(self) -> None:
        self.h = 0xCBF29CE484222325
        self.size = 0

    def add(self, data: bytes) -> "Fnv1a":
        h = self.h
        for b in data:
            h = ((h ^ b) * 0x100000001B3) & MASK64
        self.h = h
        self.size += len(data)
        return self

    def add_text(self, s: str) -> "Fnv1a":
        return self.add(s.encode("utf-8"))

    def add_u64(self, v: int) -> "Fnv1a":
        return self.add((v & MASK64).to_bytes(8, "little"))


def make_blob(n: int) -> bytes:
    return bytes((j * 31 + 7) & 0xFF for j in range(n))


def make_text(n: int) -> str:
    return "".join(chr(ord("a") + j % 26) for j in range(n))


def nat64_at(j: int) -> int:
    return (j * 0x9E3779B97F4A7C15) & MASK64


def blob_text(data: bytes) -> str:
    return 'blob "' + "".join(f"\\{b:02x}" for b in data) + '"'


def text_lit(s: str) -> str:
    return '"' + s.replace("\\", "\\\\").replace('"', '\\"') + '"'


def summary_text(i: int) -> str:
    address = f"sender{i}@example.com"
    fields = [
        ("uid", f"{i} : nat64"),
        ("state", f"{i % 3} : nat64"),
        ("flags", f"{i * 7} : nat64"),
        ("modseq", f"{1000 + i} : nat64"),
        ("received_ns", f"{1795000000000000000 + i} : nat64"),
        ("sent_at", f"{1794000000000000000 + i} : nat64"),
        ("envelope_from", text_lit(address)),
        ("subject", text_lit(f"Subject number {i} of the benchmark")),
        ("sender", text_lit(f"Sender {i}")),
        ("attachments", f"{i % 4} : nat64"),
        ("preview", text_lit("p" * 100 + str(i))),
        ("raw_bytes", f"{5000 + i} : nat64"),
        ("folder", f"{i % 5} : nat64"),
        ("sender_address", text_lit(address)),
    ]
    return "record { " + "; ".join(f"{k} = {v}" for k, v in fields) + " }"


def record10(i: int) -> Tuple[List[int], List[str]]:
    nats = [i, i * 2, i * 3, i + 1000000, i ^ 0xFFFF]
    texts = [f"name{i}", f"value{i}", "t", "", "x" * 20]
    return nats, texts


def record10_text(i: int) -> str:
    nats, texts = record10(i)
    fields = [f"f{k} = {v} : nat64" for k, v in enumerate(nats)]
    fields += [f"f{k + 5} = {text_lit(v)}" for k, v in enumerate(texts)]
    return "record { " + "; ".join(fields) + " }"


# ----------------------------------------------------------------------------
# didc


def didc_encode(value_text: str, types: str = "", method: str = "") -> bytes:
    """The bytes of didc's encoding, with the types of a type list or a
    method of the canister's .did file"""
    cmd = ["didc", "encode", "-d", str(DID_PATH)]
    cmd += ["-t", types] if types else ["-m", method]
    out = subprocess.run(
        cmd, input=value_text, capture_output=True, text=True, check=True
    )
    return bytes.fromhex(out.stdout.strip())


def fnv_of(data: bytes) -> Tuple[int, int]:
    f = Fnv1a().add(data)
    return f.size, f.h


# ----------------------------------------------------------------------------
# The cases: (id, method, argument as Candid text, expected size and hash)


def decode_smtp_request(body_size: int) -> Tuple[str, Tuple[int, int]]:
    headers = [(f"X-Header-{i}", f"value {i} " + "v" * 40) for i in range(20)]
    body = make_blob(body_size)
    headers_text = "; ".join(
        f"record {{ name = {text_lit(n)}; value = {text_lit(v)} }}" for n, v in headers
    )
    arg = (
        f"(record {{ message = opt record {{ headers = vec {{ {headers_text} }};"
        f" body = {blob_text(body)} }}; envelope = opt record {{ from = record {{"
        ' user = "a"; domain = "example.com" }; to = vec { record { user = "b";'
        ' domain = "example.com" } } }; gateway_flags = null;'
        ' message_id = opt "<id@example.com>" })'
    )
    f = Fnv1a().add(body)
    for n, v in headers:
        f.add_text(n).add_text(v)
    return arg, (f.size, f.h)


def decode_mail_send(attachment_sizes: List[int]) -> Tuple[str, Tuple[int, int]]:
    to = ["a@example.com", "b@example.com"]
    cc = ["c@example.com"]
    bcc: List[str] = []
    html = "<p>" + "h" * 1000 + "</p>"
    attachments = [
        (f"a{k}.bin", make_blob(size)) for k, size in enumerate(attachment_sizes)
    ]
    attachments_text = "; ".join(
        f"record {{ filename = {text_lit(n)}; content = {blob_text(c)} }}"
        for n, c in attachments
    )
    arg = (
        '(record { client_id = "client-1"; to = vec { "a@example.com";'
        ' "b@example.com" }; cc = vec { "c@example.com" }; bcc = vec {};'
        f' subject = "Benchmark"; html_body = {text_lit(html)};'
        f" attachments = vec {{ {attachments_text} }} }})"
    )
    f = Fnv1a().add_text("client-1")
    for s in to + cc + bcc:
        f.add_text(s)
    f.add_text("Benchmark").add_text(html)
    for n, c in attachments:
        f.add_text(n).add(c)
    return arg, (f.size, f.h)


def decode_blob(n: int) -> Tuple[str, Tuple[int, int]]:
    data = make_blob(n)
    return f"({blob_text(data)})", fnv_of(data)


def decode_text(n: int) -> Tuple[str, Tuple[int, int]]:
    s = make_text(n)
    return f"({text_lit(s)})", fnv_of(s.encode())


def decode_vec_nat64(n: int) -> Tuple[str, Tuple[int, int]]:
    values = [nat64_at(j) for j in range(n)]
    f = Fnv1a()
    for v in values:
        f.add_u64(v)
    return "(vec { " + "; ".join(str(v) for v in values) + " })", (f.size, f.h)


def decode_vec_record10(n: int) -> Tuple[str, Tuple[int, int]]:
    f = Fnv1a()
    for i in range(n):
        nats, texts = record10(i)
        for v in nats:
            f.add_u64(v)
        for s in texts:
            f.add_text(s)
    elements = "; ".join(record10_text(i) for i in range(n))
    return f"(vec {{ {elements} }})", (f.size, f.h)


def encode_expected(value_text: str, types: str) -> Tuple[int, int]:
    return fnv_of(didc_encode(value_text, types=types))


def encode_mail_list(n: int) -> Tuple[int, int]:
    items = "; ".join(summary_text(i) for i in range(n))
    return encode_expected(
        f"(record {{ items = vec {{ {items} }}; cursor = opt {n} }})", "(Page)"
    )


def encode_mail_get(n: int) -> Tuple[int, int]:
    seen = (
        'vec { record { "principal" = principal "2vxsx-fae"; name = opt "Alice";'
        ' at_ns = 1 }; record { "principal" = principal "aaaaa-aa"; name = null;'
        " at_ns = 2 } }"
    )
    value = (
        f"(record {{ summary = {summary_text(n)}; raw = {blob_text(make_blob(n))};"
        f" seen_by = {seen} }})"
    )
    return encode_expected(value, "(FullMessage)")


def encode_mail_export_chunk(n: int) -> Tuple[int, int]:
    value = (
        f"(record {{ data = {blob_text(make_blob(n))}; size = {n};"
        " started_ns = 1795000000000000000 })"
    )
    return encode_expected(value, "(ExportChunk)")


def encode_blob(n: int) -> Tuple[int, int]:
    return encode_expected(f"({blob_text(make_blob(n))})", "(blob)")


def encode_text(n: int) -> Tuple[int, int]:
    return encode_expected(f"({text_lit(make_text(n))})", "(text)")


def encode_vec_nat64(n: int) -> Tuple[int, int]:
    values = "; ".join(str(nat64_at(j)) for j in range(n))
    return encode_expected(f"(vec {{ {values} }})", "(vec nat64)")


def encode_vec_record10(n: int) -> Tuple[int, int]:
    elements = "; ".join(record10_text(i) for i in range(n))
    return encode_expected(f"(vec {{ {elements} }})", "(vec Record10)")


def encode_record(n: int) -> Tuple[int, int]:
    fields = "; ".join(f"f{i} = {i}" for i in range(n))
    types = "(record { " + "; ".join(f"f{i} : nat64" for i in range(n)) + " })"
    return encode_expected(f"(record {{ {fields} }})", types)


def encode_small(n: int) -> Tuple[int, int]:
    value = (
        f'(variant {{ Err = record {{ code = {n}; message = "mailbox not found" }} }})'
    )
    return encode_expected(value, "(SmtpError)")


# Decode cases: (case id, method, builder, its parameter)
DECODE_CASES: List[
    Tuple[str, str, Callable[..., Tuple[str, Tuple[int, int]]], object]
] = [
    ("smtp_request body 1 KB", "decode_smtp_request", decode_smtp_request, 1_000),
    ("smtp_request body 100 KB", "decode_smtp_request", decode_smtp_request, 100_000),
    ("smtp_request body 1.9 MB", "decode_smtp_request", decode_smtp_request, 1_900_000),
    ("mail_send no attachments", "decode_mail_send", decode_mail_send, []),
    ("mail_send 1 x 1 MB", "decode_mail_send", decode_mail_send, [1_000_000]),
    ("mail_send 3 x 600 KB", "decode_mail_send", decode_mail_send, [600_000] * 3),
    ("blob 1 KB", "decode_blob", decode_blob, 1_000),
    ("blob 100 KB", "decode_blob", decode_blob, 100_000),
    ("blob 1.9 MB", "decode_blob", decode_blob, 1_900_000),
    ("text 1 KB", "decode_text", decode_text, 1_000),
    ("text 100 KB", "decode_text", decode_text, 100_000),
    ("text 1.9 MB", "decode_text", decode_text, 1_900_000),
    ("vec nat64 1 KB", "decode_vec_nat64", decode_vec_nat64, 125),
    ("vec nat64 100 KB", "decode_vec_nat64", decode_vec_nat64, 12_500),
    ("vec nat64 1.9 MB", "decode_vec_nat64", decode_vec_nat64, 237_500),
    ("vec record10 x 10", "decode_vec_record10", decode_vec_record10, 10),
    ("vec record10 x 1000", "decode_vec_record10", decode_vec_record10, 1_000),
    ("vec record10 x 10000", "decode_vec_record10", decode_vec_record10, 10_000),
]

# Encode cases: (case id, method, builder of the expected bytes, n)
ENCODE_CASES: List[Tuple[str, str, Callable[[int], Tuple[int, int]], int]] = [
    ("mail_list 50", "encode_mail_list", encode_mail_list, 50),
    ("mail_list 500", "encode_mail_list", encode_mail_list, 500),
    ("mail_get raw 100 KB", "encode_mail_get", encode_mail_get, 100_000),
    ("mail_get raw 1.9 MB", "encode_mail_get", encode_mail_get, 1_900_000),
    (
        "mail_export_chunk 1.9 MB",
        "encode_mail_export_chunk",
        encode_mail_export_chunk,
        1_900_000,
    ),
    ("blob 1 KB", "encode_blob", encode_blob, 1_000),
    ("blob 100 KB", "encode_blob", encode_blob, 100_000),
    ("blob 1.9 MB", "encode_blob", encode_blob, 1_900_000),
    ("text 1 KB", "encode_text", encode_text, 1_000),
    ("text 100 KB", "encode_text", encode_text, 100_000),
    ("text 1.9 MB", "encode_text", encode_text, 1_900_000),
    ("vec nat64 1 KB", "encode_vec_nat64", encode_vec_nat64, 125),
    ("vec nat64 100 KB", "encode_vec_nat64", encode_vec_nat64, 12_500),
    ("vec nat64 1.9 MB", "encode_vec_nat64", encode_vec_nat64, 237_500),
    ("vec record10 x 10", "encode_vec_record10", encode_vec_record10, 10),
    ("vec record10 x 1000", "encode_vec_record10", encode_vec_record10, 1_000),
    ("vec record10 x 10000", "encode_vec_record10", encode_vec_record10, 10_000),
    ("record 10 fields", "encode_record", encode_record, 10),
    ("record 50 fields", "encode_record", encode_record, 50),
    ("small reply", "encode_small", encode_small, 7),
]

# ----------------------------------------------------------------------------
# Calling, checking and recording

RESULTS: Dict[str, Dict[str, int]] = {}


def parse_result(response: str) -> Dict[str, int]:
    result = {}
    for name in ("instructions", "size", "fnv1a"):
        m = re.search(rf"{name} = ([0-9_]+)", response)
        assert m, f"no {name} in the reply: {response[:500]}"
        result[name] = int(m.group(1).replace("_", ""))
    return result


def load_baseline() -> Dict[str, Dict[str, int]]:
    if not BASELINE_PATH.exists():
        return {}
    data: Dict[str, Dict[str, int]] = json.loads(
        BASELINE_PATH.read_text(encoding="utf-8")
    )
    return data


UPDATE = os.environ.get("ICPP_CANDID_BENCH_UPDATE", "")
BASELINE = load_baseline()
ALL_CASES = {f"decode {c[0]}" for c in DECODE_CASES} | {
    f"encode {c[0]}" for c in ENCODE_CASES
}


def write_baseline() -> None:
    """Writes the baseline of every case that still exists, after each case,
    so that an interrupted update keeps what it measured. Through a temporary
    file, so that an interruption leaves the old file or the new one whole"""
    kept = {case: v for case, v in BASELINE.items() if case in ALL_CASES}
    tmp = BASELINE_PATH.with_suffix(".json.tmp")
    tmp.write_text(
        json.dumps(dict(sorted(kept.items())), indent=2) + "\n", encoding="utf-8"
    )
    os.replace(tmp, BASELINE_PATH)


def check_baseline(case: str, result: Dict[str, int]) -> None:
    RESULTS[case] = result
    pct = f"{TOLERANCE:.0%}"
    used = result["instructions"]
    expected = BASELINE.get(case, {}).get("instructions")
    if expected is None:
        assert (
            UPDATE
        ), f"No baseline for '{case}': record it with ICPP_CANDID_BENCH_UPDATE=1"
    else:
        assert used <= expected * (1 + TOLERANCE) or UPDATE == "force", (
            f"'{case}' takes {used:,} instructions, more than {pct} above its"
            f" baseline of {expected:,}. If that is intended, record it with"
            " ICPP_CANDID_BENCH_UPDATE=force"
        )
        if not UPDATE and used < expected * (1 - TOLERANCE):
            warnings.warn(
                f"'{case}' takes {used:,} instructions, more than {pct} below"
                f" its baseline of {expected:,}: update the baseline with"
                " ICPP_CANDID_BENCH_UPDATE=1"
            )
    if UPDATE:
        BASELINE[case] = {"instructions": used}
        write_baseline()


def call(
    method: str, argument: str, canister_input: str, network: str
) -> Dict[str, int]:
    response = call_canister_api(
        icp_yaml_path=ICP_YAML_PATH,
        canister_name=CANISTER_NAME,
        canister_method=method,
        canister_argument=argument,
        canister_input=canister_input,
        network=network,
    )
    return parse_result(response)


@pytest.fixture(scope="module", autouse=True)
def write_results() -> Iterator[None]:
    yield
    if not RESULTS:
        return
    RESULTS_PATH.parent.mkdir(parents=True, exist_ok=True)
    RESULTS_PATH.write_text(json.dumps(RESULTS, indent=2) + "\n", encoding="utf-8")
    print("\n\n" + f"{'Case':<36} {'Instructions':>16} {'Bytes':>12}")
    for case, r in RESULTS.items():
        print(f"{case:<36} {r['instructions']:>16,} {r['size']:>12,}")


@pytest.mark.parametrize(
    "case, method, builder, param", DECODE_CASES, ids=[c[0] for c in DECODE_CASES]
)
def test__decode(
    case: str,
    method: str,
    builder: Callable[..., Tuple[str, Tuple[int, int]]],
    param: object,
    network: str,
) -> None:
    arg_text, (size, fnv) = builder(param)
    # The argument goes in as didc's bytes, the same bytes for every CDK
    arg_hex = didc_encode(arg_text, method=method).hex()
    result = call(method, arg_hex, "hex", network)
    assert (result["size"], result["fnv1a"]) == (
        size,
        fnv,
    ), f"'{method}' decoded other values than were sent"
    check_baseline(f"decode {case}", result)


@pytest.mark.parametrize(
    "case, method, builder, n", ENCODE_CASES, ids=[c[0] for c in ENCODE_CASES]
)
def test__encode(
    case: str,
    method: str,
    builder: Callable[[int], Tuple[int, int]],
    n: int,
    network: str,
) -> None:
    result = call(method, f"({n} : nat64)", "idl", network)
    # The encoded bytes must be didc's, byte for byte
    assert (result["size"], result["fnv1a"]) == builder(
        n
    ), f"'{method}' encoded other bytes than didc"
    check_baseline(f"encode {case}", result)
