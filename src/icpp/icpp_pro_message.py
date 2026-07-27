"""Fetches the message that icpp-pro prints at the end of a build"""

import json
import re
from icpp import config_default
from icpp.run_shell_cmd import run_shell_cmd

# Matches `message = "..."` in the Candid text returned by the messaging
# canister, capturing the quoted string including its escape sequences.
_MESSAGE_RE = re.compile(r'message\s*=\s*("(?:[^"\\]|\\.)*")')


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
            # Candid escapes strings the same way JSON does, so json.loads
            # turns `"a\nb"` into the two-line string we want to print.
            return str(json.loads(match.group(1)))
    except:  # pylint: disable=bare-except
        pass

    return config_default.ICPP_PRO_MESSAGING_CANISTER_DEFAULT_MESSAGE
