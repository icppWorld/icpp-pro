"""Runs a command, and kills it when its memory use goes over a limit.

The native tests (`build-native/mockic.exe`) run the canister code on the host,
where nothing caps its memory the way the IC does. A bug that allocates without
limit then fills the host's memory and swap within minutes. The guard checks the
command's resident memory every second (with `ps`, on macOS and Linux; macOS does
not enforce memory rlimits). Over the limit, it kills the command, prints an
ERROR (also as a GitHub Actions error annotation) and returns 1. When it cannot
measure, it fails the same way: it never lets a run go unguarded. Otherwise it
returns the command's own exit code.

From llama_cpp_canister's scripts/memory_guard.py (PR #62).
"""

import os
import platform
import resource
import subprocess
import sys
import time
from pathlib import Path
from typing import List, Optional

# The IC's limit for the heap of a wasm32 canister: a native run that needs
# more could never run on the IC
DEFAULT_MAX_GB = 4.0


def rss_bytes(pid: int) -> Optional[int]:
    """The resident memory of a process, from `ps`. None when the process is gone."""
    result = subprocess.run(
        ["ps", "-o", "rss=", "-p", str(pid)],
        capture_output=True,
        text=True,
        check=False,
    )
    kilobytes = result.stdout.strip()
    return int(kilobytes) * 1024 if kilobytes.isdigit() else None


def size(n: int) -> str:
    """A size in GiB, or in MiB when it is smaller than 1 GiB."""
    return f"{n / 2**30:.1f} GiB" if n >= 2**30 else f"{n / 2**20:.1f} MiB"


def children_peak_bytes() -> int:
    """The peak resident memory of the waited-for child processes so far. It
    catches the peak of a run that ends before the first check."""
    peak = resource.getrusage(resource.RUSAGE_CHILDREN).ru_maxrss
    return peak if platform.system() == "Darwin" else peak * 1024


def error(message: str) -> None:
    """Prints an ERROR line, and a GitHub Actions error annotation in CI."""
    print(f"ERROR: {message}", file=sys.stderr, flush=True)
    if os.environ.get("GITHUB_ACTIONS") == "true":
        print(f"::error title=memory guard::{message}", flush=True)


def stop(process: "subprocess.Popen[bytes]", message: str) -> int:
    """Kills the command, reports why, and returns the guard's exit code 1."""
    process.kill()
    process.wait()
    error(message)
    return 1


def guard(
    command: List[str],
    max_bytes: int,
    interval: float = 1.0,
    cwd: Optional[Path] = None,
) -> int:
    """Runs the command; kills it above max_bytes, or when its memory cannot be
    measured. Returns the exit code."""
    with subprocess.Popen(command, cwd=cwd) as process:
        peak = 0
        while process.poll() is None:
            try:
                rss = rss_bytes(process.pid)
            except OSError as e:
                return stop(process, f"cannot run ps to measure {command[0]}: {e}")
            if rss is None:
                if process.poll() is not None:
                    break  # it ended between the two checks
                return stop(process, f"cannot measure the memory of {command[0]}")
            peak = max(peak, rss)
            if rss > max_bytes:
                return stop(
                    process,
                    f"{command[0]} used {size(rss)}, over the "
                    f"{size(max_bytes)} limit of the memory guard: "
                    "killed. Something allocates without limit.",
                )
            time.sleep(interval)
    peak = max(peak, children_peak_bytes())
    print(f"[memory guard] {command[0]}: peak {size(peak)}", flush=True)
    return process.returncode
