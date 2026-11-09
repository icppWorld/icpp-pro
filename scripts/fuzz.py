"""Fuzzes icpp-candid's decoder and encoder with libFuzzer, under ASan and UBSan.

Three targets, in test/fuzz/:
  fuzz_decoder    CandidDecoder, then the text printer and parser, and
                  CandidEncoder on what it decoded
  fuzz_typed_api  CandidDeserialize into the typed API's CandidType* objects
  fuzz_encoder    CandidEncoder on malformed CandidValues

The seeds are every blob of the compliance suite (test/canisters/
canister_candid_compliance/suite), each prefixed with every value of the first
byte, which picks the expected types. Inputs that once crashed are kept in
test/fuzz/regressions/ and replayed on every run.

    python -m scripts.fuzz replay              # every seed & regression, once
    python -m scripts.fuzz run --seconds 3600  # fuzz each target for an hour

libFuzzer ships with LLVM's clang, but not with Apple's: on a Mac, install it
with `brew install llvm`. Set FUZZ_CXX to use another compiler.
"""

import argparse
import os
import shutil
import subprocess
import sys
from pathlib import Path

from scripts.candid_compliance.generate import SUITE_PATH, parse

ROOT_PATH = Path(__file__).parent.parent
FUZZ_PATH = ROOT_PATH / "test/fuzz"
BUILD_PATH = ROOT_PATH / "build-fuzz"
CANDID_PATH = ROOT_PATH / "icpp-candid/src/icpp_candid"

# Each target, and how many values its first byte picks from
TARGETS = {"fuzz_decoder": 9, "fuzz_typed_api": 6, "fuzz_encoder": 8}

FLAGS = [
    "-std=c++20",
    "-g",
    "-O1",
    "-fsanitize=fuzzer,address,undefined",
    "-fno-sanitize-recover=undefined",
    "-fno-omit-frame-pointer",
    "-D",
    "ICPP_VERBOSE=0",
]


def compiler() -> str:
    """The clang++ that has libFuzzer"""
    if "FUZZ_CXX" in os.environ:
        return os.environ["FUZZ_CXX"]
    brew = Path("/opt/homebrew/opt/llvm/bin/clang++")
    if sys.platform == "darwin" and brew.exists():
        return str(brew)
    return "clang++"


def candid_sources() -> tuple[list[str], list[str]]:
    """icpp-candid's sources, and the include flags, for a host program"""
    sources = sorted(
        [str(p) for p in (CANDID_PATH / "candid").glob("*.cpp")]
        + [str(p) for p in (CANDID_PATH / "hooks").glob("*.cpp")]
        + [str(p) for p in (CANDID_PATH / "vendors/hash-library").glob("*.cpp")]
    )
    includes = [
        f"-I{p}"
        for p in [
            FUZZ_PATH,
            CANDID_PATH / "candid",
            CANDID_PATH / "hooks",
            CANDID_PATH / "vendors",
            CANDID_PATH / "vendors/hash-library",
        ]
    ]
    return sources, includes


def build() -> None:
    """Compile the targets into build-fuzz/"""
    BUILD_PATH.mkdir(exist_ok=True)
    sources, includes = candid_sources()
    for target in TARGETS:
        print(f"--- building {target}", flush=True)
        subprocess.run(
            [compiler(), *FLAGS, *includes]
            + [str(FUZZ_PATH / f"{target}.cpp"), str(FUZZ_PATH / "fuzz_ic0.cpp")]
            + sources
            + ["-o", str(BUILD_PATH / target)],
            check=True,
        )


def seeds() -> int:
    """Write the seeds of each target into build-fuzz/seeds/<target>/"""
    blobs = []
    for f in sorted(SUITE_PATH.glob("**/*.test.did")):
        for a in parse(f.read_text(encoding="utf-8")).asserts:
            for side in (a.left, a.right):
                if side is not None and side[0] == "blob":
                    blobs.append(side[1])
    blobs = sorted(set(blobs))
    for target, picks in TARGETS.items():
        out = BUILD_PATH / "seeds" / target
        shutil.rmtree(out, ignore_errors=True)
        out.mkdir(parents=True)
        for i, blob in enumerate(blobs):
            for pick in range(picks):
                (out / f"{i:05d}-{pick}").write_bytes(bytes([pick]) + blob)
    return len(blobs)


def fuzz(target: str, extra: list[str]) -> int:
    """Run one target on its corpus, seeds and regressions"""
    corpus = BUILD_PATH / "corpus" / target
    corpus.mkdir(parents=True, exist_ok=True)
    crashes = BUILD_PATH / "crashes"
    crashes.mkdir(exist_ok=True)
    cmd = [
        str(BUILD_PATH / target),
        f"-artifact_prefix={crashes}/{target}-",
        # The memory one input may make the decoder hold is checked by
        # test/fuzz/memory_oracle.h; these catch whatever it does not see
        "-rss_limit_mb=512",
        "-malloc_limit_mb=16",
        "-timeout=20",
        "-print_final_stats=1",
        *extra,
        str(corpus),
        str(BUILD_PATH / "seeds" / target),
        str(FUZZ_PATH / "regressions"),
    ]
    # A small quarantine, so that the process's memory follows what the
    # decoder holds, under -rss_limit_mb
    env = {**os.environ, "ASAN_OPTIONS": "quarantine_size_mb=16"}
    print(f"--- {' '.join(cmd)}", flush=True)
    return subprocess.run(cmd, check=False, env=env).returncode


def main() -> int:
    """build, seed, then replay or run"""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("mode", choices=["replay", "run"])
    parser.add_argument(
        "--seconds", type=int, default=60, help="run: how long to fuzz each target"
    )
    args = parser.parse_args()

    build()
    print(f"--- seeds: {seeds()} distinct blobs of the compliance suite")
    failed = []
    for target in TARGETS:
        extra = (
            ["-runs=0"]
            if args.mode == "replay"
            else [f"-max_total_time={args.seconds}"]
        )
        if fuzz(target, extra) != 0:
            failed.append(target)
    if failed:
        print(f"ERROR: {', '.join(failed)} found a problem, see build-fuzz/crashes/")
        return 1
    print(f"--- fuzz {args.mode}: no crash")
    return 0


if __name__ == "__main__":
    sys.exit(main())
