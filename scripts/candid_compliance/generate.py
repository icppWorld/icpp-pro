"""Generate the C++ of the Candid compliance runner from .test.did files.

The official test suite of the Candid specification
(https://github.com/dfinity/candid/tree/master/test) is a set of .test.did
files: type definitions, and asserts of the forms

    assert <input> : (<types>) "<description>";    decodes at the types
    assert <input> !: (<types>) "<description>";   does not decode
    assert <input> == <input> : (<types>) "...";   both decode, to equal values
    assert <input> != <input> : (<types>) "...";   both decode, to unequal values

where an input is binary, `blob "DIDL..."`, or textual, `"(...)"`.

This script reads them, builds each assert's expected types as a Candid type
graph (named types included, so recursion works), and writes one C++ file per
.test.did file, with a function that runs every assert through a
ComplianceRunner. The runner decodes the binary inputs with CandidDecoder and
parses the textual ones with CandidTextValue.

Usage, from the root of icpp-pro:

    python scripts/candid_compliance/generate.py

It regenerates test/canisters/canister_candid_compliance/src/suite_*.cpp from
the .test.did files in that canister's suite/ folder.
"""

# pylint: disable=too-many-branches,too-many-return-statements

import subprocess
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple, Union

from icpp.config_default import CLANG_FORMAT

ROOT_PATH = Path(__file__).parent.parent.parent
CANISTER_PATH = ROOT_PATH / "test/canisters/canister_candid_compliance"
SUITE_PATH = CANISTER_PATH / "suite"
OUT_PATH = CANISTER_PATH / "src"

PRIMS = {
    "null": -1,
    "bool": -2,
    "nat": -3,
    "int": -4,
    "nat8": -5,
    "nat16": -6,
    "nat32": -7,
    "nat64": -8,
    "int8": -9,
    "int16": -10,
    "int32": -11,
    "int64": -12,
    "float32": -13,
    "float64": -14,
    "text": -15,
    "reserved": -16,
    "empty": -17,
    "principal": -24,
}

MODES = {"query": 1, "oneway": 2, "composite_query": 3}


class ParseError(Exception):
    """An error in a .test.did file"""


# --- Lexing ---


@dataclass
class Tok:
    """A token: kind is 'id', 'nat', 'str' or the punctuation itself"""

    kind: str
    value: Union[str, bytes]
    line: int


PUNCT = ["!:", "==", "!=", "->", "(", ")", "{", "}", ";", ":", ",", "=", "."]


def unescape(s: str, line: int) -> bytes:
    """The bytes of a Candid text literal's contents"""
    out = bytearray()
    i = 0
    while i < len(s):
        c = s[i]
        if c != "\\":
            out += c.encode("utf-8")
            i += 1
            continue
        e = s[i + 1]
        simple = {"t": b"\t", "n": b"\n", "r": b"\r", '"': b'"', "'": b"'"}
        if e in simple:
            out += simple[e]
            i += 2
        elif e == "\\":
            out += b"\\"
            i += 2
        elif e == "u":
            end = s.index("}", i)
            out += chr(int(s[i + 3 : end].replace("_", ""), 16)).encode("utf-8")
            i = end + 1
        else:
            try:
                out.append(int(s[i + 1 : i + 3], 16))
            except ValueError as exc:
                raise ParseError(f"line {line}: bad escape \\{s[i+1:i+3]}") from exc
            i += 3
    return bytes(out)


def lex(src: str) -> List[Tok]:
    """The tokens of a .test.did file, without comments"""
    toks: List[Tok] = []
    i = 0
    line = 1
    n = len(src)
    while i < n:
        c = src[i]
        if c == "\n":
            line += 1
            i += 1
        elif c.isspace():
            i += 1
        elif src.startswith("//", i):
            while i < n and src[i] != "\n":
                i += 1
        elif src.startswith("/*", i):
            end = src.index("*/", i)
            line += src.count("\n", i, end)
            i = end + 2
        elif c == '"':
            j = i + 1
            while src[j] != '"':
                j += 2 if src[j] == "\\" else 1
            toks.append(Tok("str", src[i + 1 : j], line))
            i = j + 1
        elif c.isalpha() or c == "_":
            j = i
            while j < n and (src[j].isalnum() or src[j] == "_"):
                j += 1
            toks.append(Tok("id", src[i:j], line))
            i = j
        elif c.isdigit():
            j = i
            while j < n and (src[j].isalnum() or src[j] == "_"):
                j += 1
            toks.append(Tok("nat", src[i:j], line))
            i = j
        else:
            for p in PUNCT:
                if src.startswith(p, i):
                    toks.append(Tok(p, p, line))
                    i += len(p)
                    break
            else:
                raise ParseError(f"line {line}: unexpected character {c!r}")
    return toks


# --- Types ---

# A type expression: ("prim", opcode) | ("name", str) | ("opt", T) |
# ("vec", T) | ("record", [(id, T)]) | ("variant", [(id, T)]) |
# ("func", [T], [T], [mode]) | ("service", [(name, T)])
Type = Tuple[Any, ...]


def idl_hash(name: bytes) -> int:
    """The field id of a name"""
    h = 0
    for b in name:
        h = (h * 223 + b) % (1 << 32)
    return h


class Parser:
    """Recursive descent over the tokens of one .test.did file"""

    def __init__(self, toks: List[Tok]):
        self.toks = toks
        self.i = 0

    def peek(self, k: int = 0) -> Optional[Tok]:
        """The token k ahead, if any"""
        j = self.i + k
        return self.toks[j] if j < len(self.toks) else None

    def kind_at(self, k: int) -> str:
        """The kind of the token k ahead, or "" at the end"""
        t = self.peek(k)
        return t.kind if t is not None else ""

    def next(self) -> Tok:
        """Consume a token"""
        t = self.peek()
        if t is None:
            raise ParseError("unexpected end of file")
        self.i += 1
        return t

    def at(self, kind: str, value: Optional[str] = None) -> bool:
        """True if the next token is of this kind (and value)"""
        t = self.peek()
        return t is not None and t.kind == kind and (value is None or t.value == value)

    def expect(self, kind: str, value: Optional[str] = None) -> Tok:
        """Consume a token that must be of this kind (and value)"""
        t = self.next()
        if t.kind != kind or (value is not None and t.value != value):
            raise ParseError(
                f"line {t.line}: expected {value or kind}, got {t.value!r}"
            )
        return t

    def label(self) -> Tuple[int, bool]:
        """A field label: nat, name or text. Returns (id, True), or (0, False)
        when the next tokens are not a label followed by ':'"""
        t, t2 = self.peek(), self.peek(1)
        if t is None or t2 is None or t2.kind != ":":
            return 0, False
        if t.kind == "nat":
            v = int(str(t.value).replace("_", ""), 0)
        elif t.kind == "id":
            v = idl_hash(str(t.value).encode("utf-8"))
        elif t.kind == "str":
            v = idl_hash(unescape(str(t.value), t.line))
        else:
            return 0, False
        self.i += 2
        return v, True

    def fields(self, variant: bool) -> List[Tuple[int, Type]]:
        """The fields of a record or variant type"""
        self.expect("{")
        out: List[Tuple[int, Type]] = []
        nxt = 0
        while not self.at("}"):
            fid, have = self.label()
            if have:
                ty = self.typ()
            elif variant and self.kind_at(1) in (";", "}"):
                # A case without a type is of type null
                t = self.next()
                if t.kind == "nat":
                    fid = int(str(t.value).replace("_", ""), 0)
                elif t.kind == "str":
                    fid = idl_hash(unescape(str(t.value), t.line))
                else:
                    fid = idl_hash(str(t.value).encode("utf-8"))
                ty = ("prim", PRIMS["null"])
            else:
                fid = nxt
                ty = self.typ()
            out.append((fid, ty))
            nxt = fid + 1
            if not self.at(";"):
                break
            self.next()
        self.expect("}")
        return out

    def tuple_type(self) -> List[Type]:
        """( T, ... ), where each T may be named: (x : T)"""
        self.expect("(")
        out: List[Type] = []
        while not self.at(")"):
            if self.at("id") and self.kind_at(1) == ":":
                self.i += 2
            out.append(self.typ())
            if not self.at(","):
                break
            self.next()
        self.expect(")")
        return out

    def func_type(self) -> Type:
        """( args ) -> ( rets ) modes"""
        args = self.tuple_type()
        self.expect("->")
        rets = self.tuple_type()
        modes = []
        while self.at("id") and str(self.toks[self.i].value) in MODES:
            modes.append(MODES[str(self.next().value)])
        return ("func", args, rets, modes)

    def typ(self) -> Type:
        """A type expression"""
        t = self.next()
        if t.kind != "id":
            raise ParseError(f"line {t.line}: expected a type, got {t.value!r}")
        v = str(t.value)
        if v in PRIMS:
            return ("prim", PRIMS[v])
        if v == "opt":
            return ("opt", self.typ())
        if v == "vec":
            return ("vec", self.typ())
        if v == "blob":
            return ("vec", ("prim", PRIMS["nat8"]))
        if v == "record":
            return ("record", self.fields(False))
        if v == "variant":
            return ("variant", self.fields(True))
        if v == "func":
            return self.func_type()
        if v == "service":
            self.expect("{")
            methods: List[Tuple[bytes, Type]] = []
            while not self.at("}"):
                nt = self.next()
                name = (
                    unescape(str(nt.value), nt.line)
                    if nt.kind == "str"
                    else str(nt.value).encode("utf-8")
                )
                self.expect(":")
                mt = self.func_type() if self.at("(") else self.typ()
                methods.append((name, mt))
                if not self.at(";"):
                    break
                self.next()
            self.expect("}")
            return ("service", methods)
        return ("name", v)

    def input(self) -> Tuple[str, bytes]:
        """blob "..." or "..." """
        if self.at("id", "blob"):
            self.next()
            t = self.expect("str")
            return "blob", unescape(str(t.value), t.line)
        t = self.expect("str")
        return "text", unescape(str(t.value), t.line)


@dataclass
class Assert:
    """One assert of a .test.did file"""

    line: int
    left: Tuple[str, bytes]
    op: str  # ':' '!:' '==' '!='
    right: Optional[Tuple[str, bytes]]
    types: List[Type]
    desc: str


@dataclass
class TestFile:
    """A parsed .test.did file"""

    defs: Dict[str, Type] = field(default_factory=dict)
    asserts: List[Assert] = field(default_factory=list)


def parse(src: str) -> TestFile:
    """Parse a .test.did file"""
    p = Parser(lex(src))
    tf = TestFile()
    while p.peek() is not None:
        kw = p.expect("id")
        if kw.value == "type":
            name = str(p.expect("id").value)
            p.expect("=")
            if name in tf.defs:
                raise ParseError(f"line {kw.line}: type {name} defined twice")
            tf.defs[name] = p.typ()
        elif kw.value == "assert":
            left = p.input()
            right = None
            op_tok = p.next()
            op = str(op_tok.value)
            if op in ("==", "!="):
                right = p.input()
                p.expect(":")
            elif op not in (":", "!:"):
                raise ParseError(f"line {kw.line}: unexpected {op!r}")
            types = p.tuple_type()
            desc = ""
            if p.at("str"):
                t = p.next()
                desc = unescape(str(t.value), t.line).decode("utf-8")
            tf.asserts.append(Assert(kw.line, left, op, right, types, desc))
        else:
            raise ParseError(f"line {kw.line}: expected type or assert")
        p.expect(";")
    return tf


# --- The graph ---


class Graph:
    """Type graph nodes, as the C++ CandidTypeGraph holds them"""

    def __init__(self, defs: Dict[str, Type]):
        self.defs = defs
        self.nodes: List[Optional[str]] = []  # C++ expressions of the nodes
        self.named: Dict[str, int] = {}
        # Named types that are constructors get a node each, up front, so that
        # they can refer to each other in any order
        for name, ty in defs.items():
            if ty[0] not in ("prim", "name"):
                self.named[name] = len(self.nodes)
                self.nodes.append(None)
        for name, idx in self.named.items():
            self.nodes[idx] = self.node(defs[name])

    def copy(self) -> "Graph":
        """A copy that shares the named types, to add an assert's types to"""
        g = Graph.__new__(Graph)
        g.defs, g.named, g.nodes = self.defs, self.named, list(self.nodes)
        return g

    def resolve(self, name: str, seen: Tuple[str, ...] = ()) -> int:
        """The reference of a named type"""
        if name in self.named:
            return self.named[name]
        if name in seen:
            raise ParseError(f"type {name} is an alias of itself")
        if name not in self.defs:
            raise ParseError(f"type {name} is not defined")
        ty = self.defs[name]
        if ty[0] == "prim":
            return int(ty[1])
        return self.resolve(str(ty[1]), seen + (name,))

    def ref(self, ty: Type) -> int:
        """The reference of a type, adding nodes as needed"""
        if ty[0] == "prim":
            return int(ty[1])
        if ty[0] == "name":
            return self.resolve(str(ty[1]))
        idx = len(self.nodes)
        self.nodes.append(None)
        self.nodes[idx] = self.node(ty)
        return idx

    def node(self, ty: Type) -> str:
        """The C++ expression of a constructor's node"""
        kind = ty[0]
        if kind in ("opt", "vec"):
            return f"cn_{kind}({self.ref(ty[1])})"
        if kind in ("record", "variant"):
            fs = sorted((fid, self.ref(t)) for fid, t in ty[1])
            body = ", ".join(f"{{{fid}u, {r}}}" for fid, r in fs)
            return f"cn_{kind}({{{body}}})"
        if kind == "func":
            args = ", ".join(str(self.ref(t)) for t in ty[1])
            rets = ", ".join(str(self.ref(t)) for t in ty[2])
            modes = ", ".join(str(m) for m in sorted(ty[3]))
            return f"cn_func({{{args}}}, {{{rets}}}, {{{modes}}})"
        if kind == "service":
            ms = sorted((name, self.ref(t)) for name, t in ty[1])
            body = ", ".join(f"{{{cpp_str(name)}, {r}}}" for name, r in ms)
            return f"cn_service({{{body}}})"
        raise ParseError(f"unknown type {ty!r}")


def cpp_str(b: bytes) -> str:
    """A C++ std::string expression holding exactly these bytes"""
    out = []
    for c in b:
        if 0x20 <= c < 0x7F and chr(c) not in '"\\?':
            out.append(chr(c))
        else:
            out.append(f"\\{c:03o}")
    return f'std::string("{"".join(out)}", {len(b)})'


def cpp_input(inp: Tuple[str, bytes]) -> str:
    """A ComplianceInput expression"""
    kind, data = inp
    return (
        f"ComplianceInput{{{'true' if kind == 'blob' else 'false'}, {cpp_str(data)}}}"
    )


OPS = {":": "Op::Decodes", "!:": "Op::Fails", "==": "Op::Equal", "!=": "Op::NotEqual"}


def generate(name: str, path: str, tf: TestFile) -> str:
    """The C++ file for one .test.did file"""
    base = Graph(tf.defs)
    fn = f"suite_{name}"
    lines = [
        f"// Generated by scripts/candid_compliance/generate.py from suite/{path}.",
        "// Do not edit: regenerate it.",
        "",
        '#include "compliance_runner.h"',
        "",
        f"void {fn}(ComplianceRunner &r) {{",
        "  CandidTypeGraph base;",
    ]
    for n in base.nodes:
        lines.append(f"  base.add({n});")
    for a in tf.asserts:
        g = base.copy()
        args = [g.ref(t) for t in a.types]
        lines.append("  {")
        lines.append("    CandidTypeGraph g = base;")
        for n in g.nodes[len(base.nodes) :]:
            lines.append(f"    g.add({n});")
        right = cpp_input(a.right) if a.right else "ComplianceInput{}"
        lines.append(
            f"    r.check({a.line}, {cpp_str(a.desc.encode('utf-8'))}, g, "
            f"{{{', '.join(str(x) for x in args)}}}, {OPS[a.op]},\n"
            f"            {cpp_input(a.left)},\n            {right});"
        )
        lines.append("  }")
    lines.append("}")
    return "\n".join(lines) + "\n"


def main() -> int:
    """Regenerate every suite file"""
    files = sorted(SUITE_PATH.glob("**/*.test.did"))
    if not files:
        print(f"ERROR: no .test.did files in {SUITE_PATH}")
        return 1
    names = []
    total = 0
    for f in files:
        name = f.name.removesuffix(".test.did").replace("-", "_")
        rel = f.parent.relative_to(SUITE_PATH)
        prefix = "_".join(rel.parts)
        name = f"{prefix}_{name}" if prefix else name
        try:
            tf = parse(f.read_text(encoding="utf-8"))
        except (ParseError, ValueError, IndexError) as e:
            print(f"ERROR in {f}: {e}")
            return 1
        path = f.relative_to(SUITE_PATH).as_posix()
        (OUT_PATH / f"suite_{name}.cpp").write_text(
            generate(name, path, tf), encoding="utf-8"
        )
        print(f"{path}: {len(tf.asserts)} asserts")
        names.append((name, path))
        total += len(tf.asserts)

    # The list of suites
    lines = [
        "// Generated by scripts/candid_compliance/generate.py. Do not edit.",
        "",
        '#include "compliance_runner.h"',
        "",
    ]
    for name, _ in names:
        lines.append(f"void suite_{name}(ComplianceRunner &r);")
    lines += [
        "",
        "const std::vector<ComplianceSuite> &compliance_suites() {",
        "  static const std::vector<ComplianceSuite> suites{",
    ]
    for name, path in names:
        lines.append(f'      {{"{path}", suite_{name}}},')
    lines += ["  };", "  return suites;", "}"]
    (OUT_PATH / "suites.cpp").write_text("\n".join(lines) + "\n", encoding="utf-8")

    # Formatted as `make cpp-format` does, so that the files are stable
    outs = [OUT_PATH / "suites.cpp"] + [OUT_PATH / f"suite_{n}.cpp" for n, _ in names]
    subprocess.run(
        [str(CLANG_FORMAT), "--style=file", "-i", *[str(o) for o in outs]],
        cwd=ROOT_PATH,
        check=True,
    )
    print(f"total: {total} asserts")
    return 0


if __name__ == "__main__":
    sys.exit(main())
