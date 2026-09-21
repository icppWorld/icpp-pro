"""Shared fixtures for icpp-pro's unit tests.

These run without building a canister, so they give a signal in seconds
instead of after a full compile + link + deploy cycle.
"""

from pathlib import Path

import pytest

FIXTURES_DIR = Path(__file__).parent / "fixtures"

# The 9.9 MB pre-optimize artifact a llama_cpp_canister build leaves behind.
# Far too large to commit, so it is used only when it happens to be present -
# real production bytes when available, the committed fixture always.
LLAMA_BEFORE_OPT = (
    Path(__file__).parent.parent.parent.parent
    / "llama_cpp_canister/build/llama_cpp_before_opt.wasm"
)


@pytest.fixture(scope="session")
def many_globals_wasm() -> Path:
    """A ~20 KB wasm with 1001 defined globals, all exported.

    Just over the IC's 1000-defined-globals limit (IC0505) and shaped like the
    production case: the exports are what keep the dead globals alive.
    Regenerate with test/unit/fixtures/generate_many_globals.py.
    """
    path = FIXTURES_DIR / "many_globals.wasm"
    assert path.is_file(), f"missing fixture {path}"
    return path


@pytest.fixture(scope="session")
def llama_before_opt_wasm() -> Path:
    """llama_cpp_canister's real pre-optimize wasm, when it is available."""
    if not LLAMA_BEFORE_OPT.is_file():
        pytest.skip(
            f"no llama artifact at {LLAMA_BEFORE_OPT} - build llama_cpp_canister "
            f"to exercise this test against real production bytes"
        )
    return LLAMA_BEFORE_OPT
