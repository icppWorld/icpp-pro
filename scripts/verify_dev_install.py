"""Assert icpp-pro & icpp-candid are editable installs of THIS working tree.

Sibling verification (`make siblings-verify-api`, `make upgrade-test-llama`)
shells out to icpp-demos and llama_cpp_canister, which run whatever `icpp` is on
the PATH. That is only a test of the dev tree if the dev tree is what is
installed.

It is easy for it not to be. Every sibling's `requirements.txt` pins icpp-pro
from PyPI, so installing those requirements after the editable install can
replace it with a released wheel:

    icpp-demos  ->  icpp-pro>=X.Y.Z   harmless while the dev version satisfies it
    llama       ->  icpp-pro==X.Y.Z   replaces the dev tree unless versions match

The failure is silent: the verification still runs and still passes, it just
proves nothing about the code being developed. This check turns that into a
loud error.
"""

import sys
from pathlib import Path

ROOT_PATH = Path(__file__).parent.parent.resolve()

# package import name -> the source tree it must resolve into
PACKAGES = {
    "icpp": ROOT_PATH / "src" / "icpp",
    "icpp_candid": ROOT_PATH / "icpp-candid" / "src" / "icpp_candid",
}


def installed_location(package: str) -> Path | None:
    """Directory the package actually imports from, or None when absent."""
    try:
        module = __import__(package)
    except ImportError:
        return None
    module_file = getattr(module, "__file__", None)
    if module_file is None:
        return None
    return Path(module_file).resolve().parent


def main() -> int:
    """Verify every package resolves into this working tree."""
    failures = []
    for package, expected in PACKAGES.items():
        actual = installed_location(package)
        if actual is None:
            failures.append(f"{package}: not importable at all")
        elif actual != expected:
            failures.append(
                f"{package}: imports from {actual}\n      expected {expected}"
            )
        else:
            print(f"  ✅ {package}: editable, this tree")

    if failures:
        print("\n❌ the environment is NOT running this working tree:")
        for failure in failures:
            print(f"  - {failure}")
        print(
            "\nA sibling's requirements.txt pins icpp-pro from PyPI and can\n"
            "replace the editable install. Repair the environment with:\n\n"
            "    make install-python\n\n"
            "or, when you need a sibling's dependencies too:\n\n"
            "    make install-python-w-demos\n"
            "    make install-python-w-llama_cpp_canister\n"
        )
        return 1

    print("✅ dev install verified: icpp-pro & icpp-candid are this working tree")
    return 0


if __name__ == "__main__":
    sys.exit(main())
