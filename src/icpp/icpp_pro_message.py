"""Fetches the message that icpp-pro prints at the end of a build"""

import re
from icpp import config_default
from icpp.run_shell_cmd import run_shell_cmd

# Matches `message = "..."` in the Candid text returned by the messaging
# canister, capturing the quoted string including its escape sequences.
_MESSAGE_RE = re.compile(r'message\s*=\s*("(?:[^"\\]|\\.)*")')

# The escapes in a Candid text literal: a run of UTF-8 bytes as `\e2\9c\93`,
# or one `\u{fe0f}` (hex digits, with an optional `_` between two), `\n`, ...
_ESCAPE_RE = re.compile(
    r"((?:\\[0-9a-fA-F]{2})+)|\\(u\{[0-9a-fA-F](?:_?[0-9a-fA-F])*\}|.)"
)
_ESCAPES = {"n": "\n", "r": "\r", "t": "\t", "\\": "\\", '"': '"', "'": "'"}


def _unescape(match: re.Match[str]) -> str:
    hex_bytes, escape = match.groups()
    if hex_bytes:
        return bytes.fromhex(hex_bytes.replace("\\", "")).decode("utf-8")
    if escape.startswith("u{"):
        code_point = int(escape[2:-1].replace("_", ""), 16)
        if 0xD800 <= code_point <= 0xDFFF:
            raise ValueError(f"\\{escape} is a surrogate, not a character")
        return chr(code_point)
    return _ESCAPES[escape]


def candid_text(quoted: str) -> str:
    """Returns the text of a quoted Candid text literal, e.g. `"a\\nb"`.

    Candid writes a character it does not print as `\\u{X}`, e.g. `\\u{fe0f}`
    after an emoji, which is not JSON. Raises on an escape it does not know,
    or on a code point that is not a character: a surrogate, or above U+10FFFF.
    """
    return _ESCAPE_RE.sub(_unescape, quoted[1:-1])


def icpp_pro_message() -> str:
    """Returns the message of the icpp-pro messaging canister on the IC.

    Best effort: falls back to the default message when the IC cannot be
    reached, when icp-cli is not installed, or when the response cannot be
    parsed. A build must never fail over a cosmetic message.
    """
    try:
        cmd = (
            f"icp canister call {config_default.ICPP_PRO_MESSAGING_CANISTER} "
            f"get_message '()' --network ic --query --output candid < /dev/null"
        )
        response_str = run_shell_cmd(cmd, capture_output=True)
        match = _MESSAGE_RE.search(response_str)
        if match is not None:
            return candid_text(match.group(1))
    except:  # pylint: disable=bare-except
        pass

    return config_default.ICPP_PRO_MESSAGING_CANISTER_DEFAULT_MESSAGE
