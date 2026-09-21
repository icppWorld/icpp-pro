"""A demo post_wasm_function, and the ordering-contract test.

icpp-pro applies its built-in globals-limit fix (IC0505) BEFORE calling this
hook, so what a hook receives is always an already-valid wasm. This demo
asserts exactly that, which makes the canister a regression test for the
ordering contract rather than just a "the build did not crash" check.

It also saves its own `<canister>_before_opt.wasm`, the way a hand-written
hook conventionally does. That is safe: the built-in step keeps its backup
under `_before_opt_internal`, so the two never overwrite each other and both
copies survive the build.
"""

import shutil

from icpp import icpp_toml

from icpp.fix_globals_step import BACKUP_SUFFIX


def main() -> None:
    """Asserts the built-in fix already ran, then keeps its own backup."""

    build_path = icpp_toml.icpp_toml_path.parent / "build"
    wasm_path = (build_path / f"{icpp_toml.build_wasm['canister']}.wasm").resolve()
    internal_backup = wasm_path.with_name(
        wasm_path.stem + BACKUP_SUFFIX + wasm_path.suffix
    ).resolve()

    if not wasm_path.is_file():
        raise RuntimeError(f"post_wasm_function: no wasm at {wasm_path}")

    # The ordering contract: wasi2ic -> built-in fix -> THIS HOOK -> objcopy.
    if not internal_backup.is_file():
        raise RuntimeError(
            f"post_wasm_function ran BEFORE the built-in globals-limit fix: "
            f"expected {internal_backup} to exist already"
        )

    # The hook sees the fixed wasm, not the pre-optimize one.
    if wasm_path.read_bytes() == internal_backup.read_bytes():
        raise RuntimeError(
            "post_wasm_function received the pre-optimize wasm; it must receive "
            "the output of the built-in globals-limit fix"
        )

    # A hook's own backup, under the conventional name. It does not collide
    # with the built-in one.
    hook_backup = wasm_path.with_name(
        wasm_path.stem + "_before_opt" + wasm_path.suffix
    ).resolve()
    shutil.copy(wasm_path, hook_backup)

    print(f"post_wasm_function: the built-in fix already ran on {wasm_path}")
    print(f"post_wasm_function: its backup is {internal_backup}")
    print(f"post_wasm_function: this hook also saved {hook_backup}")


if __name__ == "__main__":
    # For debugging without running `icpp build-wasm`,
    # (-) make sure to run this from the root folder, as:
    #     python -m scripts.optimize_wasm
    #     -> That way, import icpp_toml works correctly and
    #        all values will be set
    #
    main()
