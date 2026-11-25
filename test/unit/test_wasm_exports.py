"""The export check of `icpp build-wasm`, on wasm modules built in the test.

The IC refuses a module with more than 1100 exports. The canister tests check
the exports of real builds (test_apis.py of canister_1)."""

# pylint: disable=missing-function-docstring

from pathlib import Path
from typing import Sequence, Tuple, Union

import pytest

from icpp import config_default
from icpp.wasm_exports import (
    MAX_EXPORTS,
    MAX_METHOD_NAME_LENGTHS,
    MAX_METHODS,
    EXPORT_IF_DEFINED,
    SYSTEM_FUNCTIONS,
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


def _wasm(exports: Sequence[Tuple[Union[str, bytes], int]]) -> bytes:
    """A module with a custom section and an export section. Only the export
    section is read, so the indices need not point anywhere."""
    payload = _leb128(len(exports))
    for index, (name, kind) in enumerate(exports):
        encoded = name.encode("utf-8") if isinstance(name, str) else name
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


def test__the_limits_are_allowed(tmp_path: Path) -> None:
    # The most methods, the longest names, and the most exports, all at once
    name_length = MAX_METHOD_NAME_LENGTHS // MAX_METHODS
    exports = [
        (f"canister_update {i:0{name_length}d}", FUNC) for i in range(MAX_METHODS)
    ]
    exports += [(f"_Z{i}", FUNC) for i in range(MAX_EXPORTS - MAX_METHODS)]
    summary = check_export_limit(_write(tmp_path, _wasm(exports)))
    assert summary == (
        f"Exports: {MAX_EXPORTS}, of which {MAX_METHODS} methods "
        f"(the IC allows {MAX_EXPORTS} and {MAX_METHODS})"
    )


def test__one_over_the_limit_fails(tmp_path: Path) -> None:
    exports = [("memory", MEMORY)] + [(f"_Z{i}", FUNC) for i in range(MAX_EXPORTS)]
    with pytest.raises(WasmExportsError) as e:
        check_export_limit(_write(tmp_path, _wasm(exports)))
    message = str(e.value)
    assert f"has {MAX_EXPORTS + 1} exports" in message
    assert f"more than {MAX_EXPORTS}" in message
    assert f"{MAX_EXPORTS} of them are neither entry points nor the memory" in message
    assert "e.g. _Z0, _Z1, _Z2, _Z3, _Z4\n" in message
    assert "cpp_link_flags" in message and "post_wasm_function" in message


def test__too_many_entry_points_say_so(tmp_path: Path) -> None:
    exports = [(f"canister_query m{i}", FUNC) for i in range(MAX_METHODS)]
    exports += [
        (f"canister_update u{i}", FUNC) for i in range(MAX_EXPORTS - MAX_METHODS + 1)
    ]
    with pytest.raises(WasmExportsError) as e:
        check_export_limit(_write(tmp_path, _wasm(exports)))
    assert "All of them are entry points: the canister has too many." in str(e.value)
    assert "--export-dynamic" not in str(e.value)


@pytest.mark.parametrize(
    "prefix", ["canister_query ", "canister_composite_query ", "canister_update "]
)
def test__one_method_over_the_limit_fails(tmp_path: Path, prefix: str) -> None:
    exports = [(f"{prefix}m{i}", FUNC) for i in range(MAX_METHODS + 1)]
    with pytest.raises(WasmExportsError) as e:
        check_export_limit(_write(tmp_path, _wasm(exports)))
    assert f"has {MAX_METHODS + 1} methods (query, composite query" in str(e.value)


def test__other_entry_points_are_not_methods(tmp_path: Path) -> None:
    exports = [(f"canister_query m{i}", FUNC) for i in range(MAX_METHODS)]
    exports += [("canister_init", FUNC), ("canister_global_timer", FUNC)]
    assert f"of which {MAX_METHODS} methods" in check_export_limit(
        _write(tmp_path, _wasm(exports))
    )


def test__method_names_one_byte_too_long_fail(tmp_path: Path) -> None:
    # Only the name after the prefix counts, in bytes: "é" is 2
    half = MAX_METHOD_NAME_LENGTHS // 4
    exports = [("canister_query " + "é" * half, FUNC)]
    exports += [("canister_update " + "x" * (MAX_METHOD_NAME_LENGTHS - 2 * half), FUNC)]
    assert check_export_limit(_write(tmp_path, _wasm(exports)))
    exports.append(("canister_update y", FUNC))
    with pytest.raises(WasmExportsError) as e:
        check_export_limit(_write(tmp_path, _wasm(exports)))
    assert f"{MAX_METHOD_NAME_LENGTHS + 1} bytes long together" in str(e.value)


def test__unknown_canister_names_over_the_limit_are_named(tmp_path: Path) -> None:
    exports = [(f"canister_querry m{i}", FUNC) for i in range(MAX_EXPORTS + 1)]
    with pytest.raises(WasmExportsError) as e:
        check_export_limit(_write(tmp_path, _wasm(exports)))
    assert "e.g. canister_querry m0, canister_querry m1" in str(e.value)


def test__a_missing_wasm_fails(tmp_path: Path) -> None:
    with pytest.raises(WasmExportsError, match="cannot read"):
        export_names(tmp_path / "missing.wasm")


def test__a_file_that_is_not_wasm_fails(tmp_path: Path) -> None:
    with pytest.raises(WasmExportsError, match="is not a wasm module"):
        export_names(_write(tmp_path, b"not a wasm module"))


def test__a_truncated_wasm_fails(tmp_path: Path) -> None:
    data = _wasm([("canister_init", FUNC)])
    with pytest.raises(WasmExportsError, match="the wasm ends inside a section"):
        export_names(_write(tmp_path, data[:-3]))


def test__every_system_function_is_allowed(tmp_path: Path) -> None:
    exports = [("canister_query greet", FUNC)] + [(n, FUNC) for n in SYSTEM_FUNCTIONS]
    summary = check_export_limit(_write(tmp_path, _wasm(exports)))
    assert "WARNING" not in summary


@pytest.mark.parametrize(
    "name", ["canister_querry greet", "canister_query", "canister_start", "canister_"]
)
def test__an_unknown_canister_export_fails(tmp_path: Path, name: str) -> None:
    exports = [("canister_query greet", FUNC), (name, FUNC)]
    with pytest.raises(WasmExportsError) as e:
        check_export_limit(_write(tmp_path, _wasm(exports)))
    assert f"exports {name}, and the IC refuses a canister_ export" in str(e.value)


@pytest.mark.parametrize("kind", [TABLE, MEMORY, GLOBAL])
def test__a_canister_export_that_is_no_function_fails(
    tmp_path: Path, kind: int
) -> None:
    exports = [("canister_query greet", FUNC), ("canister_query data", kind)]
    with pytest.raises(WasmExportsError, match="or that is not a function"):
        check_export_limit(_write(tmp_path, _wasm(exports)))


def test__a_method_under_two_call_types_fails(tmp_path: Path) -> None:
    exports = [("canister_query get", FUNC), ("canister_update get", FUNC)]
    with pytest.raises(WasmExportsError, match="exports the method get twice"):
        check_export_limit(_write(tmp_path, _wasm(exports)))


def test__a_wasm_without_methods_warns(tmp_path: Path) -> None:
    exports = [("memory", MEMORY), ("canister_global_timer", FUNC)]
    summary = check_export_limit(_write(tmp_path, _wasm(exports)))
    assert "WARNING: the wasm exports no query, composite query or update" in summary


def test__the_link_exports_every_system_function() -> None:
    # Also one a canister defines without WASM_SYMBOL_EXPORTED
    flags = config_default.WASM_LD_REQUIRED_FLAGS
    assert "-Wl,--export=canister_global_timer" in flags
    for name in EXPORT_IF_DEFINED:
        assert f"-Wl,--export-if-defined={name} " in flags
    # Every other system function, except canister_pre_upgrade: the flag would
    # pull the weak default of ic_async.cpp into every canister
    rest = set(SYSTEM_FUNCTIONS) - set(EXPORT_IF_DEFINED)
    assert rest == {"canister_global_timer", "canister_pre_upgrade"}
    assert "canister_pre_upgrade" not in flags


def test__an_invalid_utf8_name_counts_its_bytes(tmp_path: Path) -> None:
    # Decoded, each 0xff byte would become a 3-byte U+FFFD
    exports = [(b"canister_query " + b"\xff" * MAX_METHOD_NAME_LENGTHS, FUNC)]
    assert check_export_limit(_write(tmp_path, _wasm(exports)))


def test__an_export_past_its_section_fails(tmp_path: Path) -> None:
    # The name's length says 9 bytes, and the section ends after 4
    section = b"\x01" + b"\x09" + b"init"
    data = b"\x00asm\x01\x00\x00\x00" + _section(7, section) + _section(0, b"\x01x")
    with pytest.raises(WasmExportsError, match="runs past the export section"):
        export_names(_write(tmp_path, data))


def test__the_link_does_not_export_every_symbol() -> None:
    # --export-dynamic exported every libc++ and polyfill symbol, over 2000
    assert "--export-dynamic" not in config_default.WASM_LD_REQUIRED_FLAGS
    wasm_symbol_h = (config_default.ICPP_PATH / "ic/icapi/wasm_symbol.h").read_text(
        encoding="utf-8"
    )
    assert "export_name(name)" in wasm_symbol_h
