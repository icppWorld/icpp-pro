"""The message printed at the end of a build decodes Candid's text escapes.

Regression: the messaging canister's reply was parsed with json.loads, which
rejects Candid's `\\u{fe0f}`. A message with an emoji variation selector then
printed the default message instead.
"""

# pylint: disable=missing-function-docstring

import subprocess
from typing import Callable

import pytest

from icpp import config_default, icpp_pro_message
from icpp.icpp_pro_message import candid_text

DEFAULT = config_default.ICPP_PRO_MESSAGING_CANISTER_DEFAULT_MESSAGE

# The reply on 2026-11-02, as `icp canister call ... --output candid` prints it
REPLY = """(
  variant {
    Ok = record {
      message = "Thank you for using icpp-pro 💻\\u{fe0f}\\nDocs: https://icpp-pro.onicai.com";
    }
  },
)
"""


def replying(reply: str) -> Callable[..., str]:
    def fake_run_shell_cmd(_cmd: str, **_kwargs: object) -> str:
        return reply

    return fake_run_shell_cmd


def test__the_live_reply_prints_with_its_emoji_and_newline(
    monkeypatch: pytest.MonkeyPatch,
) -> None:
    monkeypatch.setattr(icpp_pro_message, "run_shell_cmd", replying(REPLY))
    assert icpp_pro_message.icpp_pro_message() == (
        "Thank you for using icpp-pro 💻\ufe0f\nDocs: https://icpp-pro.onicai.com"
    )


@pytest.mark.parametrize(
    "quoted, text",
    [
        (r'"say \"hi\""', 'say "hi"'),
        (r'"C:\\icpp"', "C:\\icpp"),
        (r'"a\tb\rc\'d"', "a\tb\rc'd"),
        (r'"\u{1F4BB}"', "💻"),
        ('"plain"', "plain"),
    ],
)
def test__candid_text_decodes_escapes(quoted: str, text: str) -> None:
    assert candid_text(quoted) == text


@pytest.mark.parametrize("quoted", [r'"\x"', r'"\u{110000}"'])
def test__an_escape_it_does_not_know_raises(quoted: str) -> None:
    with pytest.raises((KeyError, ValueError)):
        candid_text(quoted)


def test__an_unknown_escape_prints_the_default_message(
    monkeypatch: pytest.MonkeyPatch,
) -> None:
    reply = '(variant { Ok = record { message = "a \\x b" } })'
    monkeypatch.setattr(icpp_pro_message, "run_shell_cmd", replying(reply))
    assert icpp_pro_message.icpp_pro_message() == DEFAULT


def test__a_failed_call_prints_the_default_message(
    monkeypatch: pytest.MonkeyPatch,
) -> None:
    def failing(_cmd: str, **_kwargs: object) -> str:
        raise subprocess.CalledProcessError(1, "icp")

    monkeypatch.setattr(icpp_pro_message, "run_shell_cmd", failing)
    assert icpp_pro_message.icpp_pro_message() == DEFAULT


def test__an_error_reply_prints_the_default_message(
    monkeypatch: pytest.MonkeyPatch,
) -> None:
    reply = "(variant { Err = variant { StatusCode = 403 : nat16 } })"
    monkeypatch.setattr(icpp_pro_message, "run_shell_cmd", replying(reply))
    assert icpp_pro_message.icpp_pro_message() == DEFAULT
