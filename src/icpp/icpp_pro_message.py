"""Fetches the message that icpp-pro prints at the end of a build"""

import re
from icpp import config_default
from icpp.run_shell_cmd import run_shell_cmd

# Matches `message = "..."` in the Candid text returned by the messaging
# canister, capturing the quoted string including its escape sequences.
_MESSAGE_RE = re.compile(r'message\s*=\s*("(?:[^"\\]|\\.)*")')

# One escape sequence in a Candid text literal: `\u{fe0f}` or `\n`, `\"`, ...
_ESCAPE_RE = re.compile(r"\\(u\{[0-9a-fA-F]{1,6}\}|.)")
_ESCAPES = {"n": "\n", "r": "\r", "t": "\t", "\\": "\\", '"': '"', "'": "'"}


def _unescape(match: re.Match[str]) -> str:
    escape = match.group(1)
    if escape.startswith("u{"):
        return chr(int(escape[2:-1], 16))
    return _ESCAPES[escape]


def candid_text(quoted: str) -> str:
    """Returns the text of a quoted Candid text literal, e.g. `"a\\nb"`.

    Candid writes a character it does not print as `\\u{X}`, e.g. `\\u{fe0f}`
    after an emoji, which is not JSON. Raises on an escape it does not know,
    or on a code point above U+10FFFF.
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
