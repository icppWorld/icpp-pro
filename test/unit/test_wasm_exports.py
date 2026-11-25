"""The export check of `icpp build-wasm`, on wasm modules built in the test.

The IC refuses a module with more than 1100 exports. The canister tests check
the exports of real builds (test_apis.py of canister_1)."""

# pylint: disable=missing-function-docstring

from pathlib import Path
from typing import List, Tuple

import pytest

from icpp import config_default
from icpp.wasm_exports import (
    MAX_EXPORTS,
    WasmExportsError,
    check_export_limit,
    export_names,
)

# The kinds of an export
FUNC, TABLE, MEMORY, GLOBAL = 0, 1, 2, 3


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


def _section(section_id: int, payload: bytes) -> bytes:
    return bytes([section_id]) + _leb128(len(payload)) + payload


def _wasm(exports: List[Tuple[str, int]]) -> bytes:
    """A module with a custom section and an export section. Only the export
    section is read, so the indices need not point anywhere."""
    payload = _leb128(len(exports))
    for index, (name, kind) in enumerate(exports):
        encoded = name.encode("utf-8")
        payload += _leb128(len(encoded)) + encoded + bytes([kind]) + _leb128(index)
    custom = _section(0, _leb128(4) + b"note" + b"x" * 200)
    return b"\x00asm\x01\x00\x00\x00" + custom + _section(7, payload)


def _write(tmp_path: Path, data: bytes) -> Path:
    path = tmp_path / "my_canister.wasm"
    path.write_bytes(data)
    return path


def test__export_names_reads_every_kind(tmp_path: Path) -> None:
    exports = [
        ("canister_query greet", FUNC),
        ("memory", MEMORY),
        ("__indirect_function_table", TABLE),
        ("__stack_pointer", GLOBAL),
        ("canister_init", FUNC),
    ]
    assert export_names(_write(tmp_path, _wasm(exports))) == [n for n, _ in exports]


def test__a_module_without_exports_has_none(tmp_path: Path) -> None:
    assert not export_names(_write(tmp_path, b"\x00asm\x01\x00\x00\x00"))


def test__the_limit_is_allowed(tmp_path: Path) -> None:
    exports = [(f"canister_query m{i}", FUNC) for i in range(MAX_EXPORTS)]
    summary = check_export_limit(_write(tmp_path, _wasm(exports)))
    assert summary == f"Exports: {MAX_EXPORTS} (the IC allows {MAX_EXPORTS})"


def test__one_over_the_limit_fails(tmp_path: Path) -> None:
    exports = [("memory", MEMORY)] + [(f"_Z{i}", FUNC) for i in range(MAX_EXPORTS)]
    with pytest.raises(WasmExportsError) as e:
        check_export_limit(_write(tmp_path, _wasm(exports)))
    message = str(e.value)
    assert f"has {MAX_EXPORTS + 1} exports" in message
    assert f"more than {MAX_EXPORTS}" in message
    assert f"{MAX_EXPORTS + 1} of them are not canister_ entry points" in message
    assert "memory, _Z0, _Z1, _Z2, _Z3\n" in message


def test__a_file_that_is_not_wasm_fails(tmp_path: Path) -> None:
    with pytest.raises(WasmExportsError, match="is not a wasm module"):
        export_names(_write(tmp_path, b"not a wasm module"))


def test__a_truncated_wasm_fails(tmp_path: Path) -> None:
    data = _wasm([("canister_init", FUNC)])
    with pytest.raises(WasmExportsError, match="the wasm ends inside a section"):
        export_names(_write(tmp_path, data[:-3]))


def test__the_link_does_not_export_every_symbol() -> None:
    # --export-dynamic exported every libc++ and polyfill symbol, over 2000
    assert "--export-dynamic" not in config_default.WASM_LD_REQUIRED_FLAGS
    wasm_symbol_h = (config_default.ICPP_PATH / "ic/icapi/wasm_symbol.h").read_text(
        encoding="utf-8"
    )
    assert "export_name(name)" in wasm_symbol_h
