"""Run a dfx command"""
import sys
import subprocess
import platform
from typing import Optional
from icpp.run_shell_cmd import run_shell_cmd


DFX = "dfx"
RUN_IN_POWERSHELL = False
if platform.win32_ver()[0]:
    DFX = "wsl --% dfx"
    RUN_IN_POWERSHELL = True


def run_dfx_cmd(args: str) -> Optional[str]:
    """Runs dfx command as a subprocess"""
    try:
        cmd = f"{DFX} {args} "
        return run_shell_cmd(
            cmd,
            capture_output=True,
            run_in_powershell=RUN_IN_POWERSHELL,
        ).rstrip("\n")
    except subprocess.CalledProcessError as e:
        print(f"Failed dfx command: '{cmd}' with error: \n{e.output}")
        sys.exit(1)
    return None
