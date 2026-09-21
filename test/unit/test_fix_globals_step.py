"""The built-in globals-limit fix, tested without building a canister.

This is the EARLY signal: if the icpp-binaryen wiring breaks, these fail in
milliseconds rather than after a full build + deploy of every test canister.

What is covered here is icpp-pro's WIRING - that the step runs on the right
artifact with the right backup semantics. The optimizer's own correctness is
icpp-binaryen's responsibility and is covered by its test suite.
"""

import shutil
from pathlib import Path

from icpp_binaryen import GlobalsFixReport

from icpp.fix_globals_step import run_fix_globals_limit

# The IC rejects a module with more than this many defined globals (IC0505).
IC_GLOBALS_LIMIT = 1000


def _fix_a_copy(wasm: Path, tmp_path: Path) -> tuple[Path, Path, GlobalsFixReport]:
    """Copies `wasm` into tmp_path and runs the step on the copy."""
    work = tmp_path / wasm.name
    shutil.copy(wasm, work)
    report = run_fix_globals_limit(work)
    backup = work.with_name(work.stem + "_before_opt" + work.suffix)
    return work, backup, report


def test__drops_below_the_ic_globals_limit(
    many_globals_wasm: Path, tmp_path: Path
) -> None:
    """A module over the IC's limit comes out under it."""
    _, _, report = _fix_a_copy(many_globals_wasm, tmp_path)

    # The fixture must actually cross the limit, or this test proves nothing.
    assert report.globals_before > IC_GLOBALS_LIMIT
    assert report.globals_after <= IC_GLOBALS_LIMIT
    assert report.globals_after < report.globals_before
    # The global exports are what keep the dead globals alive.
    assert report.exports_after < report.exports_before


def test__writes_a_backup_identical_to_the_input(
    many_globals_wasm: Path, tmp_path: Path
) -> None:
    """The _before_opt backup is the untouched pre-optimize wasm."""
    work, backup, _ = _fix_a_copy(many_globals_wasm, tmp_path)

    # The backup is the pre-optimize wasm, kept because the optimizer strips
    # the name section: it is what gives NAMED functions in a backtrace.
    assert backup.is_file()
    assert backup.read_bytes() == many_globals_wasm.read_bytes()
    assert work.read_bytes() != many_globals_wasm.read_bytes()


def test__second_run_clobbers_the_backup_which_is_why_it_runs_once(
    many_globals_wasm: Path, tmp_path: Path
) -> None:
    """Characterizes the hazard that makes `fix_globals_limit = false` exist.

    Applying the fix twice is semantically a no-op - the second run reports
    the same globals count - but it is NOT harmless: it copies the
    ALREADY-FIXED wasm over the _before_opt backup, silently destroying the
    name section the backup exists to preserve.

    icpp-pro therefore applies the step exactly once per build, and a project
    whose own post_wasm_function already runs the fix must opt out with
    `fix_globals_limit = false`.

    Note what is deliberately NOT asserted: that the wasm bytes move on the
    second run. They do on a large real module (llama's artifact was measured
    changing on run 2 and converging on run 3), but this small fixture reaches
    a fixed point immediately. Byte-stability across a re-run is therefore
    artifact-dependent and must not be relied on; the backup damage is
    universal.
    """
    work, backup, first = _fix_a_copy(many_globals_wasm, tmp_path)
    after_first = work.read_bytes()

    second = run_fix_globals_limit(work)

    # Semantically a no-op...
    assert second.globals_after == first.globals_after
    # ...yet the backup no longer holds the pre-optimize wasm.
    assert backup.read_bytes() != many_globals_wasm.read_bytes()
    assert backup.read_bytes() == after_first


def test__llama_real_artifact(llama_before_opt_wasm: Path, tmp_path: Path) -> None:
    """The same assertions against real production bytes, when available."""
    _, backup, report = _fix_a_copy(llama_before_opt_wasm, tmp_path)

    assert report.globals_before > IC_GLOBALS_LIMIT
    assert report.globals_after <= IC_GLOBALS_LIMIT
    assert backup.read_bytes() == llama_before_opt_wasm.read_bytes()
