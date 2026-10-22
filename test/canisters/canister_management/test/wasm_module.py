"""A tiny wasm module, written byte by byte, to install into a child canister.

Its canister_init and canister_post_upgrade print a message with
ic0.debug_print (a line in the canister log), and it has the custom sections
"icp:public icpp" and "icp:private icpp_secret" (canister_metadata).
"""

from typing import List


def leb(n: int) -> bytes:
    """An unsigned LEB128"""
    out = bytearray()
    while True:
        byte = n & 0x7F
        n >>= 7
        if n:
            out.append(byte | 0x80)
        else:
            out.append(byte)
            return bytes(out)


def name(s: str) -> bytes:
    return leb(len(s)) + s.encode()


def section(section_id: int, content: bytes) -> bytes:
    return bytes([section_id]) + leb(len(content)) + content


def vec(items: List[bytes]) -> bytes:
    return leb(len(items)) + b"".join(items)


def wasm_module(message: str = "hi", public: str = "public-data") -> bytes:
    """The module; message is printed by canister_init & canister_post_upgrade"""
    data = message.encode()
    types = vec(
        [
            b"\x60\x00\x00",  # 0: () -> ()
            b"\x60\x02\x7f\x7f\x00",  # 1: (i32, i32) -> ()
        ]
    )
    imports = vec([name("ic0") + name("debug_print") + b"\x00" + leb(1)])
    functions = vec([leb(0)])  # function 1: type 0
    memory = vec([b"\x00" + leb(1)])  # 1 page, no maximum
    exports = vec(
        [
            name("canister_init") + b"\x00" + leb(1),
            name("canister_post_upgrade") + b"\x00" + leb(1),
        ]
    )
    # i32.const 0; i32.const len; call 0; end
    body = b"\x00" + b"\x41\x00" + b"\x41" + leb(len(data)) + b"\x10\x00" + b"\x0b"
    code = vec([leb(len(body)) + body])
    data_section = vec([b"\x00" + b"\x41\x00\x0b" + leb(len(data)) + data])
    return (
        b"\x00asm\x01\x00\x00\x00"
        + section(1, types)
        + section(2, imports)
        + section(3, functions)
        + section(5, memory)
        + section(7, exports)
        + section(10, code)
        + section(11, data_section)
        + section(0, name("icp:public icpp") + public.encode())
        + section(0, name("icp:private icpp_secret") + b"secret")
    )
