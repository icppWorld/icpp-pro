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

from icpp.fix_globals_step import BACKUP_SUFFIX, run_fix_globals_limit

# The IC rejects a module with more than this many defined globals (IC0505).
IC_GLOBALS_LIMIT = 1000


def _fix_a_copy(wasm: Path, tmp_path: Path) -> tuple[Path, Path, GlobalsFixReport]:
    """Copies `wasm` into tmp_path and runs the step on the copy."""
    work = tmp_path / wasm.name
    shutil.copy(wasm, work)
    report = run_fix_globals_limit(work)
    backup = work.with_name(work.stem + BACKUP_SUFFIX + work.suffix)
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


def test__does_not_collide_with_a_hooks_own_backup(
    many_globals_wasm: Path, tmp_path: Path
) -> None:
    """The built-in backup and a hook's own `_before_opt` both survive.

    A post_wasm_function conventionally saves the wasm it received as
    `<stem>_before_opt.wasm`. The built-in step deliberately uses a different
    name, so the two do not fight over one file: before the suffixes were
    split, the hook's copy landed on top of ours and replaced the
    pre-optimize wasm with already-fixed bytes - silently losing the name
    section the backup exists for, while leaving a file in place so nothing
    looked wrong.
    """
    work, internal_backup, _ = _fix_a_copy(many_globals_wasm, tmp_path)

    # A hook that saves what it was handed, the conventional way.
    hook_backup = work.with_name(work.stem + "_before_opt" + work.suffix)
    shutil.copy(work, hook_backup)

    assert internal_backup != hook_backup
    # Ours still holds the PRE-optimize wasm...
    assert internal_backup.read_bytes() == many_globals_wasm.read_bytes()
    # ...and the hook's holds the POST-fix wasm it actually received.
    assert hook_backup.read_bytes() == work.read_bytes()


def test__llama_real_artifact(llama_before_opt_wasm: Path, tmp_path: Path) -> None:
    """The same assertions against real production bytes, when available."""
    _, backup, report = _fix_a_copy(llama_before_opt_wasm, tmp_path)

    assert report.globals_before > IC_GLOBALS_LIMIT
    assert report.globals_after <= IC_GLOBALS_LIMIT
    assert backup.read_bytes() == llama_before_opt_wasm.read_bytes()
