"""Checks that icpp-candid's decoder reads the message only in two functions.

The message is untrusted input. CandidDecoder keeps it in m_p, and reads it
only in Decoder::read_byte and Decoder::read_bytes, which check that the bytes
are there. Any other use of m_p is a read that may go past the end, so this
fails on it.

    python -m scripts.check_decoder_reads
"""

import re
import sys
from pathlib import Path

DECODER = (
    Path(__file__).parent.parent
    / "icpp-candid/src/icpp_candid/candid/candid_decoder.cpp"
)

# The functions that may read m_p
READERS = ("bool Decoder::read_byte(", "bool Decoder::read_bytes(")

# The member itself, and its initialisation
ALLOWED = (
    re.compile(r"^\s*const uint8_t \*m_p;\s*$"),
    re.compile(r"^\s*: m_p\(data\),"),
)


def strip_comments(text: str) -> str:
    """The C++ source without // and /* */ comments, line numbers kept"""
    text = re.sub(
        r"/\*.*?\*/", lambda m: "\n" * m.group().count("\n"), text, flags=re.S
    )
    return re.sub(r"//[^\n]*", "", text)


def main() -> int:
    """Lists every use of m_p outside the readers"""
    lines = strip_comments(DECODER.read_text(encoding="utf-8")).splitlines()
    bad = []
    in_reader = False
    for number, line in enumerate(lines, start=1):
        if line.startswith(READERS):
            in_reader = True
        if re.search(r"\bm_p\b", line) and not in_reader:
            if not any(a.search(line) for a in ALLOWED):
                bad.append(f"{DECODER.name}:{number}: {line.strip()}")
        if in_reader and line.startswith("}"):
            in_reader = False
    if bad:
        print("ERROR: the decoder reads the message outside read_byte and read_bytes:")
        print("\n".join(bad))
        return 1
    print("check-decoder-reads: the message is read only by read_byte and read_bytes")
    return 0


if __name__ == "__main__":
    sys.exit(main())
