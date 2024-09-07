"""
A demo post_wasm_function.
"""
from pathlib import Path
import shutil
from icpp import icpp_toml


def main() -> None:
    """Dummy optimization of binary wasm file"""

    build_path = icpp_toml.icpp_toml_path.parent / "build"
    wasm_path = (build_path / f"{icpp_toml.build_wasm['canister']}.wasm").resolve()

    # save the original version
    wasm_path_orig = wasm_path.with_name(
        wasm_path.stem + "_before_opt" + wasm_path.suffix
    ).resolve()
    shutil.copy(wasm_path, wasm_path_orig)

    # optimize the wasm
    # Do nothing...

    # summarize the optimization result
    print(f"We pretended to optimize the wasm: {wasm_path}")
    print(f"We saved the original version too: {wasm_path_orig}")


if __name__ == "__main__":
    # For debugging without running `icpp build-wasm`,
    # (-) make sure to run this from the root folder, as:
    #     python -m scripts.optimize_wasm
    #     -> That way, import icpp_toml works correctly and
    #        all values will be set
    #
    main()
