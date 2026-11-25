"""The export check of `icpp build-wasm`.

The IC refuses a wasm module with more than 1100 exports of any kind
(functions, globals, memories, tables), on install and on every call to an
installed canister once the replica compiles it again. See dfinity/ic PR
#12100, `MAX_NUMBER_EXPORTS` in rs/config/src/embedders.rs. The check covers
the IC's two older export limits too: the number of methods, and the length
of their names.

This reads the export section without any dependency, so the check also runs
when a project opts out of the globals-limit fix.
"""

from pathlib import Path
from typing import List, Tuple

# The IC's limits, in rs/config/src/embedders.rs of dfinity/ic: exports of any
# kind; query, composite query and update methods; the summed byte lengths of
# those methods' names, without the "canister_query " etc. prefix
MAX_EXPORTS = 1100
MAX_METHODS = 1000
MAX_METHOD_NAME_LENGTHS = 20000

_METHOD_PREFIXES = (
    b"canister_query ",
    b"canister_composite_query ",
    b"canister_update ",
)
_WASM_MAGIC = b"\x00asm"
_EXPORT_SECTION_ID = 7


class WasmExportsError(Exception):
    """The wasm cannot be read, or breaks one of the IC's export limits."""


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


def _read_export_section(section: bytes) -> List[bytes]:
    """The names in an export section; raises when an entry runs past its end."""
    count, p = _read_leb128(section, 0)
    names: List[bytes] = []
    for _ in range(count):
        length, p = _read_leb128(section, p)
        if p + length + 1 > len(section):  # the name, then the kind byte
            raise WasmExportsError("an export runs past the export section")
        names.append(section[p : p + length])
        _, p = _read_leb128(section, p + length + 1)  # the index
    return names


def _export_name_bytes(wasm_path: Path) -> List[bytes]:
    """The names of all exports of the wasm module, as the bytes they are."""
    data = wasm_path.read_bytes()
    if data[:4] != _WASM_MAGIC:
        raise WasmExportsError(f"{wasm_path} is not a wasm module")
    pos = 8  # after the magic number and the version
    while pos < len(data):
        section_id = data[pos]
        size, pos = _read_leb128(data, pos + 1)
        end = pos + size
        if end > len(data):
            raise WasmExportsError("the wasm ends inside a section")
        if section_id == _EXPORT_SECTION_ID:
            return _read_export_section(data[pos:end])
        pos = end
    return []


def export_names(wasm_path: Path) -> List[str]:
    """The names of all exports of the wasm module, of any kind."""
    return [n.decode("utf-8", errors="replace") for n in _export_name_bytes(wasm_path)]


def stray_exports(names: List[str]) -> List[str]:
    """The exports that are neither a canister_ entry point nor the memory."""
    return [n for n in names if not n.startswith("canister_") and n != "memory"]


def check_export_limit(wasm_path: Path) -> str:
    """Raises WasmExportsError when the wasm breaks one of the IC's export limits.

    Returns a one-line summary of the counts otherwise.
    """
    raw_names = _export_name_bytes(wasm_path)
    names = [n.decode("utf-8", errors="replace") for n in raw_names]
    # The query, composite query and update methods, without their prefix
    methods = [n.split(b" ", 1)[1] for n in raw_names if n.startswith(_METHOD_PREFIXES)]
    name_lengths = sum(len(m) for m in methods)
    if len(names) > MAX_EXPORTS:
        others = stray_exports(names)
        hint = (
            f"{len(others)} of them are neither canister_ entry points nor the "
            f"memory, e.g. {', '.join(others[:5])}\n"
            f"       Export entry points only with WASM_SYMBOL_EXPORTED, and do "
            f"not link with -Wl,--export-dynamic"
            if others
            else "All of them are entry points: the canister has too many."
        )
        raise WasmExportsError(
            f"{wasm_path} has {len(names)} exports, and the IC refuses a wasm "
            f"with more than {MAX_EXPORTS}. {hint}"
        )
    if len(methods) > MAX_METHODS:
        raise WasmExportsError(
            f"{wasm_path} has {len(methods)} query and update methods, and the "
            f"IC refuses a wasm with more than {MAX_METHODS}."
        )
    if name_lengths > MAX_METHOD_NAME_LENGTHS:
        raise WasmExportsError(
            f"The names of the {len(methods)} query and update methods of "
            f"{wasm_path} are {name_lengths} bytes long together, and the IC "
            f"refuses a wasm with more than {MAX_METHOD_NAME_LENGTHS}."
        )
    return (
        f"Exports: {len(names)}, of which {len(methods)} query and update methods "
        f"(the IC allows {MAX_EXPORTS} and {MAX_METHODS})"
    )
