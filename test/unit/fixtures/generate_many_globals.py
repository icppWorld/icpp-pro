"""Regenerates many_globals.wasm, the fixture for the globals-limit fix.

The committed fixture is a ~20 KB wasm with 1001 defined globals, every one of
them exported - just over the IC's 1000-defined-globals limit (install error
IC0505), and shaped like the production case: it is the exports that keep the
otherwise-dead globals alive, so removing them lets the optimizer drop all but
the two that `sum`/`bump` actually use.

The wasm is committed rather than generated at test time, so the test suite
needs no wat2wasm and runs anywhere. Only regeneration needs wasmtime:

    pip install wasmtime
    python test/unit/fixtures/generate_many_globals.py
"""

from pathlib import Path

import wasmtime

N_GLOBALS = 1001
OUT = Path(__file__).parent / "many_globals.wasm"


def _leb128(value: int) -> bytes:
    out = bytearray()
    while True:
        byte = value & 0x7F
        value >>= 7
        if value:
            out.append(byte | 0x80)
        else:
            out.append(byte)
            return bytes(out)


def _custom_section(name: str, payload: bytes) -> bytes:
    body = _leb128(len(name)) + name.encode("utf-8") + payload
    return bytes([0]) + _leb128(len(body)) + body


def _name_section(module_name: str) -> bytes:
    """A minimal wasm `name` custom section (module-name subsection only).

    The optimizer strips this; the _before_opt backup is what preserves it,
    which is how a developer gets NAMED functions in a wasmtime backtrace.
    """
    sub_payload = _leb128(len(module_name)) + module_name.encode("utf-8")
    subsection = bytes([0]) + _leb128(len(sub_payload)) + sub_payload
    return _custom_section("name", subsection)


def _target_features_section(*features: str) -> bytes:
    """A `target_features` custom section, as wasi-sdk builds carry it.

    Binaryen applies it to the module's feature set on read; without it the
    exported mutable globals are invalid under bare MVP.
    """
    payload = _leb128(len(features))
    for feature in features:
        payload += b"+" + _leb128(len(feature)) + feature.encode("utf-8")
    return _custom_section("target_features", payload)


def build_wat(n_globals: int = N_GLOBALS) -> str:
    """The .wat source: n_globals mutable globals, all exported."""
    lines = ["(module", '  (memory (export "memory") 1)']
    lines += [f"  (global $g{i} (mut i32) (i32.const {i}))" for i in range(n_globals)]
    lines += [f'  (export "g{i}" (global $g{i}))' for i in range(n_globals)]
    lines += [
        '  (func $sum (export "sum") (result i32)',
        "    (i32.add (global.get $g0) (global.get $g1)))",
        # Writes keep g0/g1 truly mutable, so the optimizer cannot fold them away
        '  (func $bump (export "bump")',
        "    (global.set $g0 (i32.add (global.get $g0) (i32.const 1)))",
        "    (global.set $g1 (i32.add (global.get $g1) (i32.const 1))))",
        ")",
    ]
    return "\n".join(lines)


def main() -> None:
    """Writes the fixture wasm."""
    wasm = bytes(wasmtime.wat2wasm(build_wat()))
    wasm += _target_features_section("mutable-globals")
    wasm += _name_section("many_globals_fixture")
    OUT.write_bytes(wasm)
    print(f"wrote {OUT} ({len(wasm):,} bytes, {N_GLOBALS} globals)")


if __name__ == "__main__":
    main()
