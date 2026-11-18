"""IC_Call::unbounded_wait warns at compile time, unless the call site passes
IC_TrustedCallee{} or the build defines ICPP_ALLOW_UNBOUNDED_WAIT."""

# pylint: disable=missing-function-docstring

import shlex
import shutil
import subprocess
from pathlib import Path

import pytest

from icpp import config_default

WARNING = "IC_Call::unbounded_wait: while this call is in flight"

pytestmark = pytest.mark.skipif(
    shutil.which(config_default.NATIVE_CPP) is None,
    reason=f"{config_default.NATIVE_CPP} is not installed",
)


def compile_call(
    tmp_path: Path, call: str, flags: str = ""
) -> "subprocess.CompletedProcess[str]":
    """Compiles a function that returns call."""
    source = tmp_path / "call.cpp"
    source.write_text(
        '#include "ic_api.h"\n'
        "IC_Call f(const CandidTypePrincipal &callee) {\n"
        f"  return {call};\n"
        "}\n",
        encoding="utf-8",
    )
    cmd = [
        config_default.NATIVE_CPP,
        *shlex.split(config_default.NATIVE_CPP_REQUIRED_FLAGS),
        *shlex.split(flags),
        "-fsyntax-only",
        str(source),
    ]
    return subprocess.run(cmd, capture_output=True, text=True, check=False)


def compile_warnings(tmp_path: Path, call: str, flags: str = "") -> str:
    """The compiler's output for a call that must compile."""
    result = compile_call(tmp_path, call, flags)
    assert result.returncode == 0, result.stderr
    return result.stderr


def test__unbounded_wait_warns(tmp_path: Path) -> None:
    assert WARNING in compile_warnings(tmp_path, 'IC_Call::unbounded_wait(callee, "m")')


def test__trusted_callee_does_not_warn(tmp_path: Path) -> None:
    assert WARNING not in compile_warnings(
        tmp_path, 'IC_Call::unbounded_wait(callee, "m", IC_TrustedCallee{})'
    )


def test__bare_braces_do_not_compile(tmp_path: Path) -> None:
    # The acknowledgement must be named, so it can be found
    result = compile_call(tmp_path, 'IC_Call::unbounded_wait(callee, "m", {})')
    assert result.returncode != 0
    # Refused for the braces, not for another error: only the overload that
    # takes the acknowledgement names it
    assert "IC_TrustedCallee" in result.stderr, result.stderr


def test__allow_unbounded_wait_does_not_warn(tmp_path: Path) -> None:
    assert WARNING not in compile_warnings(
        tmp_path,
        'IC_Call::unbounded_wait(callee, "m")',
        "-DICPP_ALLOW_UNBOUNDED_WAIT",
    )
