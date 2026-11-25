"""The export check of `icpp build-wasm`.

The IC refuses a wasm module with more than 1100 exports of any kind
(functions, globals, memories, tables), on install and on every call to an
installed canister once the replica compiles it again. See dfinity/ic PR
#12100, `MAX_NUMBER_EXPORTS` in rs/config/src/embedders.rs.

This reads the export section without any dependency, so the check also runs
when a project opts out of the globals-limit fix.
"""

from pathlib import Path
from typing import List, Tuple

# The IC's limit of exports of any kind
MAX_EXPORTS = 1100

_WASM_MAGIC = b"\x00asm"


class WasmExportsError(Exception):
    """The wasm cannot be read, or has more exports than the IC allows."""


_EXPORT_SECTION_ID = 7


def _read_leb128(data: bytes, pos: int) -> Tuple[int, int]:
    """Reads an unsigned LEB128 number; returns it and the position after it."""
    result = 0
    shift = 0
    while True:
        if pos >= len(data):
            raise WasmExportsError("the wasm ends inside a number")
        byte = data[pos]
        pos += 1
        result |= (byte & 0x7F) << shift
        if byte < 0x80:
            return result, pos
        shift += 7


def export_names(wasm_path: Path) -> List[str]:
    """The names of all exports of the wasm module, of any kind."""
    data = wasm_path.read_bytes()
    if data[:4] != _WASM_MAGIC:
        raise WasmExportsError(f"{wasm_path} is not a wasm module")
    names: List[str] = []
    pos = 8  # after the magic number and the version
    while pos < len(data):
        section_id = data[pos]
        size, pos = _read_leb128(data, pos + 1)
        end = pos + size
        if end > len(data):
            raise WasmExportsError("the wasm ends inside a section")
        if section_id == _EXPORT_SECTION_ID:
            count, p = _read_leb128(data, pos)
            for _ in range(count):
                length, p = _read_leb128(data, p)
                names.append(data[p : p + length].decode("utf-8", errors="replace"))
                p += length + 1  # the name, then the kind byte
                _, p = _read_leb128(data, p)  # the index
        pos = end
    return names


def check_export_limit(wasm_path: Path) -> str:
    """Raises WasmExportsError when the wasm has more exports than the IC allows.

    Returns a one-line summary of the count otherwise.
    """
    names = export_names(wasm_path)
    if len(names) > MAX_EXPORTS:
        others = [n for n in names if not n.startswith("canister_")]
        raise WasmExportsError(
            f"{wasm_path} has {len(names)} exports, and the IC refuses a wasm "
            f"with more than {MAX_EXPORTS}. {len(others)} of them are not "
            f"canister_ entry points, e.g. {', '.join(others[:5])}\n"
            f"       Export entry points only with WASM_SYMBOL_EXPORTED, and "
            f"do not link with -Wl,--export-dynamic"
        )
    return f"Exports: {len(names)} (the IC allows {MAX_EXPORTS})"
