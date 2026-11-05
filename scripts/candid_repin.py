"""Re-pin the expected Candid messages of tests to the encoder's bytes.

Since 7.1.0rc2 icpp-candid builds the type table at serialize time, in the
order of the Rust implementation and without unused entries, so a message with
a composite type has new bytes. The values do not change. This script finds the
hex messages in test sources and rewrites the ones that are expected outputs:

  - C++: the expected output of mockIC.run_test(name, fn, input, expected, ...),
    and a literal compared with == or !=
  - Python: a literal compared with `response ==`, and the `expected` column of
    a @pytest.mark.parametrize table

Inputs are never rewritten: a message in the old form is still valid Candid.
Every other literal that would change is listed, to look at by hand.

Each new message is the old one re-encoded by icpp-candid's encoder
(scripts/candid_repin.cpp), and checked with didc: `didc decode` of the old
and the new bytes must print the same values.

Usage:
    python -m scripts.candid_repin [--write] [paths...]

Without paths, it scans icpp-pro's test canisters and the greet template.
Without --write, it only reports.
"""

import argparse
import re
import shutil
import subprocess
import sys
from pathlib import Path

from scripts.fuzz import BUILD_PATH, CANDID_PATH, FUZZ_PATH

ROOT_PATH = Path(__file__).parent.parent
TOOL = BUILD_PATH / "candid_repin"

DEFAULT_PATHS = [
    ROOT_PATH / "test/canisters",
    ROOT_PATH / "src/icpp/canisters/greet",
]

# A message: a string literal starting with DIDL in hex, and in C++ the
# adjacent literals it continues in
LITERAL = re.compile(
    r"""(?P<q>["'])(?P<first>4449444c[0-9a-fA-F]*)(?P=q)"""
    r"""(?P<rest>(?:\s*(?P=q)[0-9a-fA-F]+(?P=q))*)"""
)


def build() -> None:
    """Compile the re-encode tool"""
    BUILD_PATH.mkdir(exist_ok=True)
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
    subprocess.run(
        ["clang++", "-std=c++20", "-O1", "-D", "ICPP_VERBOSE=0", *includes]
        + [str(ROOT_PATH / "scripts/candid_repin.cpp"), str(FUZZ_PATH / "fuzz_ic0.cpp")]
        + sources
        + ["-o", str(TOOL)],
        check=True,
    )


def reencode(messages: list[str]) -> list[str]:
    """'OK <hex>' or 'ERR <why>' per message"""
    out = subprocess.run(
        [str(TOOL)],
        input="\n".join(messages) + "\n",
        capture_output=True,
        text=True,
        check=True,
    ).stdout.splitlines()
    assert len(out) == len(messages)
    return out


def didc_decode(message: str) -> str | None:
    """The values didc reads from a message, or None"""
    r = subprocess.run(
        ["didc", "decode", message], capture_output=True, text=True, check=False
    )
    return r.stdout if r.returncode == 0 else None


def skip_string(text: str, i: int) -> int:
    """The index after the string literal that starts at i"""
    q = text[i]
    i += 1
    while i < len(text) and text[i] != q:
        i += 2 if text[i] == "\\" else 1
    return i + 1


def top_level_index(text: str, open_paren: int, pos: int, python: bool) -> int:
    """Which top-level argument of the call opened at open_paren holds pos,
    or -1 if pos is not inside it"""
    depth = 0
    index = 0
    i = open_paren
    comment = "#" if python else "//"
    while i < pos:
        c = text[i]
        if c in "\"'":
            i = skip_string(text, i)
            continue
        if text.startswith(comment, i):
            i = text.find("\n", i)
            if i < 0:
                return -1
            continue
        if c in "([{":
            depth += 1
        elif c in ")]}":
            depth -= 1
            if depth == 0:
                return -1
        elif c == "," and depth == 1:
            index += 1
        i += 1
    return index if depth >= 1 else -1


def enclosing_open(text: str, pos: int) -> int:
    """The index of the bracket that encloses pos, or -1"""
    depth = 0
    i = pos - 1
    while i >= 0:
        c = text[i]
        if c in ")]}":
            depth += 1
        elif c in "([{":
            if depth == 0:
                return i
            depth -= 1
        i -= 1
    return -1


def is_expected_python(text: str, start: int) -> bool:
    """Is the literal at start of a Python file an expected output?"""
    before = text[max(0, start - 200) : start].rstrip()
    if re.search(r"(response\s*==|expected\w*\s*=\s*\(?)$", before):
        return True
    # The expected column of a parametrize table
    p = text.rfind("@pytest.mark.parametrize(", 0, start)
    if p < 0:
        return False
    m = re.match(r"""@pytest.mark.parametrize\(\s*["']([^"']*)["']""", text[p:])
    if not m:
        return False
    names = [n.strip() for n in m.group(1).split(",")]
    row = enclosing_open(text, start)
    if "expected" not in names or row < 0 or text[row] != "(":
        return False
    idx = top_level_index(text, row, start, True)
    table = enclosing_open(text, row)
    in_table = (
        table >= 0 and text[table] == "[" and text.rfind("@pytest", 0, table) == p
    )
    return in_table and idx == names.index("expected")


def is_expected(text: str, start: int, python: bool) -> bool:
    """Is the literal at start an expected output?"""
    if python:
        return is_expected_python(text, start)
    before = text[max(0, start - 200) : start].rstrip()
    if re.search(r"(==|!=)$", before):
        return True
    # check_eq(actual, expected, what): a literal is never the actual
    call = text.rfind("check_eq(", 0, start)
    if call >= 0 and top_level_index(text, call + len("check_eq"), start, python) >= 1:
        return True
    # An array of expected arguments: check_eq(...last_call_arg_hex(...), args[i]
    brace = enclosing_open(text, start)
    if brace >= 0 and text[brace] == "{":
        m = re.search(r"(\w+)\[\]\s*=\s*$", text[:brace])
        if m and re.search(
            r"last_call_arg_hex\([^;]*\),\s*" + m.group(1) + r"\[", text
        ):
            return True
    call = text.rfind("run_test(", 0, start)
    if call >= 0:
        idx = top_level_index(text, call + len("run_test"), start, python)
        return idx == 3
    return False


def literal_hex(m: re.Match[str]) -> str:
    """The hex digits of a matched literal"""
    return m.group("first") + "".join(re.findall(r"[0-9a-fA-F]+", m.group("rest")))


def files(paths: list[Path]) -> list[Path]:
    """The .cpp and .py files of the paths, outside build folders"""
    out: list[Path] = []
    for p in paths:
        if p.is_file():
            out.append(p)
        else:
            out += [
                f
                for f in sorted(p.rglob("*"))
                if f.suffix in (".cpp", ".py")
                and "build" not in f.parts[-3:-1]
                and not any(part.startswith("build") for part in f.parts)
            ]
    return out


def main() -> int:
    """Re-pin the expected messages of the files"""
    parser = argparse.ArgumentParser(description=__doc__.split("\n", maxsplit=1)[0])
    parser.add_argument("--write", action="store_true", help="rewrite the files")
    parser.add_argument("paths", nargs="*", type=Path)
    args = parser.parse_args()
    if shutil.which("didc") is None:
        print("didc is not installed (cargo install didc)", file=sys.stderr)
        return 1
    build()

    rewritten = 0
    review: list[str] = []
    failed = 0
    for f in files(args.paths or DEFAULT_PATHS):
        text = f.read_text()
        python = f.suffix == ".py"
        matches = list(LITERAL.finditer(text))
        if not matches:
            continue
        olds = [literal_hex(m).lower() for m in matches]
        news = reencode(olds)
        edits: list[tuple[int, int, str]] = []
        for m, old, new in zip(matches, olds, news):
            if not new.startswith("OK ") or new[3:] == old:
                continue
            new = new[3:]
            line = text.count("\n", 0, m.start()) + 1
            rel = f.relative_to(ROOT_PATH) if f.is_relative_to(ROOT_PATH) else f
            where = f"{rel}:{line}"
            if not is_expected(text, m.start(), python):
                review.append(f"{where}: input or unknown, left as is: {old}")
                continue
            if didc_decode(old) != didc_decode(new) or didc_decode(new) is None:
                print(f"{where}: didc reads other values from {new}", file=sys.stderr)
                failed += 1
                continue
            q = m.group("q")
            edits.append((m.start(), m.end(), f"{q}{new}{q}"))
            print(f"{where}: {old}\n{' ' * len(where)}  -> {new}")
        if args.write and edits:
            for s, e, r in reversed(edits):
                text = text[:s] + r + text[e:]
            f.write_text(text)
        rewritten += len(edits)

    print(
        f"\n{rewritten} expected messages {'rewritten' if args.write else 'to rewrite'}"
    )
    if review:
        print(f"\n{len(review)} other messages would change; look at them by hand:")
        print("\n".join(review))
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
