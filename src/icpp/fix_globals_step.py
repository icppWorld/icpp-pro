"""The built-in globals-limit fix, a standard step of `icpp build-wasm`.

The IC rejects a wasm module with more than 1000 DEFINED globals (install
error IC0505). A wasi-sdk build of a large C++ project blows past that: what
keeps the dead globals alive is that they are exported, so removing the global
exports and letting Binaryen's optimizer drop them brings the count down.

This lives in its own module, rather than inline in commands_build_wasm.py, so
it can be tested against a wasm without running a full compile + link. See
test/unit/test_fix_globals_step.py.

Ordering contract (documented for canister authors):

    wasi2ic  ->  THIS STEP  ->  post_wasm_function  ->  objcopy metadata

It runs after wasi2ic because that is the artifact whose globals matter, and
before the user's hook so a project doing custom passes receives an
already-valid wasm as its input.
"""

from pathlib import Path
from typing import TYPE_CHECKING

if TYPE_CHECKING:  # imported for typing only - see the lazy import below
    from icpp_binaryen import GlobalsFixReport


class FixGlobalsError(Exception):
    """The built-in globals-limit fix could not run."""


def run_fix_globals_limit(wasm_path: Path) -> "GlobalsFixReport":
    """Removes exported globals & optimizes, so the wasm stays under IC0505.

    Rewrites `wasm_path` in place and writes a `<stem>_before_opt.wasm`
    backup next to it. Returns icpp_binaryen's GlobalsFixReport.

    The backup keeps the wasm name section, which the optimizer strips, so a
    developer can run the pre-optimize wasm under wasmtime and get NAMED
    functions in a backtrace. `_before_opt` is a convention other tooling
    depends on - never override backup_suffix.
    """
    # Imported here, not at module scope: icpp_binaryen raises ImportError at
    # import time on an unsupported platform (it bundles a shared library for
    # macOS and linux x86_64 only) and on a bundled-Binaryen version mismatch.
    # A module-level import would turn those into a failure of every `icpp`
    # invocation, including `icpp --version`.
    try:
        from icpp_binaryen import (  # pylint: disable=import-outside-toplevel
            fix_globals_limit,
        )
    except ImportError as e:
        raise FixGlobalsError(
            f"cannot import icpp_binaryen, which provides the built-in "
            f"globals-limit fix: {e}\n"
            f"       Install it with: pip install icpp-binaryen\n"
            f"       Or opt out in icpp.toml with:\n"
            f"         [build-wasm]\n"
            f"         fix_globals_limit = false"
        ) from e

    try:
        return fix_globals_limit(wasm_path)
    except Exception as e:  # pylint: disable=broad-except
        raise FixGlobalsError(
            f"the built-in globals-limit fix failed on {wasm_path}: {e}\n"
            f"       Opt out in icpp.toml with:\n"
            f"         [build-wasm]\n"
            f"         fix_globals_limit = false"
        ) from e
