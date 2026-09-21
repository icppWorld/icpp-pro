"""A demo post_wasm_function, and the ordering-contract test.

icpp-pro applies its built-in globals-limit fix (IC0505) BEFORE calling this
hook, so what a hook receives is always an already-valid wasm. This demo
asserts exactly that, which makes the canister a regression test for the
ordering contract rather than just a "the build did not crash" check.

Note what this hook deliberately does NOT do: write its own
`<canister>_before_opt.wasm`. The built-in step owns that backup. A hook that
writes it too would copy the already-fixed wasm over it and silently destroy
the wasm name section the backup exists to preserve.
"""

from pathlib import Path

from icpp import icpp_toml


def main() -> None:
    """Asserts the built-in globals-limit fix already ran on this wasm."""

    build_path = icpp_toml.icpp_toml_path.parent / "build"
    wasm_path = (build_path / f"{icpp_toml.build_wasm['canister']}.wasm").resolve()
    backup_path = wasm_path.with_name(
        wasm_path.stem + "_before_opt" + wasm_path.suffix
    ).resolve()

    if not wasm_path.is_file():
        raise RuntimeError(f"post_wasm_function: no wasm at {wasm_path}")

    # The ordering contract: wasi2ic -> built-in fix -> THIS HOOK -> objcopy.
    if not backup_path.is_file():
        raise RuntimeError(
            f"post_wasm_function ran BEFORE the built-in globals-limit fix: "
            f"expected the backup {backup_path} to exist already"
        )

    # The hook sees the fixed wasm, not the pre-optimize one.
    if wasm_path.read_bytes() == backup_path.read_bytes():
        raise RuntimeError(
            "post_wasm_function received the pre-optimize wasm; it must receive "
            "the output of the built-in globals-limit fix"
        )

    print(f"post_wasm_function: the built-in fix already ran on {wasm_path}")
    print(f"post_wasm_function: its pre-optimize backup is {backup_path}")


if __name__ == "__main__":
    # For debugging without running `icpp build-wasm`,
    # (-) make sure to run this from the root folder, as:
    #     python -m scripts.optimize_wasm
    #     -> That way, import icpp_toml works correctly and
    #        all values will be set
    #
    main()
