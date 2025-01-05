"""Run a dfx command"""

import sys
import subprocess
from pathlib import Path
from typing import Optional
from icpp.run_shell_cmd import run_shell_cmd


DFX = "dfx"


def run_dfx_cmd(
    args: str,
    capture_output: bool = False,
    print_captured_output: bool = False,
    cwd: Optional[Path] = None,
    timeout_seconds: Optional[int] = None,
) -> Optional[str]:
    """Runs dfx command as a subprocess"""
    try:
        cmd = f"{DFX} {args} "
        return run_shell_cmd(
            cmd,
            capture_output=capture_output,
            print_captured_output=print_captured_output,
            cwd=cwd,
            timeout_seconds=timeout_seconds,
        ).rstrip("\n")
    except subprocess.CalledProcessError as e:
        print(f"Failed dfx command: '{cmd}' with error: \n{e.output}")
        sys.exit(1)
    return None
