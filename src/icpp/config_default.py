"""
Set the default config variables
"""
import sys
import platform
from pathlib import Path
import getpass
from icpp import __version_wasi_sdk__

# The OS we're running on - https://stackoverflow.com/a/1857/5480536

OS_SYSTEM = platform.system()
OS_PROCESSOR = platform.processor()


#######################################################################
# The compatible wasi-sdk compiler


def get_wasi_sdk_os_name() -> str:
    """Returns the os name used for the releases build by the CI/CD of the wasi-sdk
    repo, for the current OS."""

    if OS_SYSTEM == "Linux":
        return "linux"

    if OS_SYSTEM == "Darwin":
        return "macos"

    return "unknown"


WASI_SDK_OS_NAME = get_wasi_sdk_os_name()
WASI_SDK_URL = (
    f"https://github.com/WebAssembly/wasi-sdk/releases/download/"
    f"{__version_wasi_sdk__.split('.',1)[0]}/"
    f"{__version_wasi_sdk__}-{WASI_SDK_OS_NAME}.tar.gz"
)
ICPP_ROOT = Path.home() / ".icpp"
ICPP_ROOT_COMPILER = ICPP_ROOT / f"{__version_wasi_sdk__}"

USER = getpass.getuser()
ICPP_PATH = Path(sys.modules["icpp"].__path__[0])
ICPP_DIR = str(ICPP_PATH)

########################################################################
# init
#
ICPP_INIT_PATH = ICPP_PATH / "canisters/greet"
LOCAL_INIT_PATH = Path("./greet")

########################################################################
# build-wasm
#
IC_C_FILES = (
    " ".join(
        [
            str(x)
            for x in list(ICPP_PATH.glob("ic/ic0/*.c"))
            + list(ICPP_PATH.glob("ic/wasi_sdk_traps/*.c"))
            + list(ICPP_PATH.glob("ic/candid/*.c"))
            + list(ICPP_PATH.glob("ic/icapi/*.c"))
            + list(ICPP_PATH.glob("ic/vendors/*.c"))
        ]
    )
    + " "
)
IC_CPP_FILES = (
    " ".join(
        [
            str(x)
            for x in list(ICPP_PATH.glob("ic/ic0/*.cpp"))
            + list(ICPP_PATH.glob("ic/candid/*.cpp"))
            + list(ICPP_PATH.glob("ic/icapi/*.cpp"))
            + list(ICPP_PATH.glob("ic/vendors/*.cpp"))
        ]
    )
    + " "
)
IC_HEADER_FILES = (
    " ".join(
        [
            str(x)
            for x in list(ICPP_PATH.glob("ic/ic0/*.h"))
            + list(ICPP_PATH.glob("ic/wasi_sdk_traps/*.h"))
            + list(ICPP_PATH.glob("ic/candid/*.h"))
            + list(ICPP_PATH.glob("ic/icapi/*.h"))
            + list(ICPP_PATH.glob("ic/vendors/*.h"))
            + list(ICPP_PATH.glob("ic/vendors/*.hpp"))
        ]
    )
    + " "
)

SYSROOT = ICPP_ROOT_COMPILER / "share/wasi-sysroot"

WASM_C = ICPP_ROOT_COMPILER / "bin/clang"
WASM_CPP = ICPP_ROOT_COMPILER / "bin/clang++"
WASM_CFLAGS = (
    f" --target=wasm32-wasi -O3 -flto -fno-exceptions -fvisibility=hidden "
    f" --sysroot {SYSROOT} "
    f" -I {ICPP_DIR}/ic/candid -I {ICPP_DIR}/ic/icapi -I {ICPP_DIR}/ic/ic0  "
    f" -I {ICPP_DIR}/ic/vendors "
    f" -D NDEBUG -D ICPP_VERBOSE=0 "
)
WASM_CPPFLAGS = WASM_CFLAGS + " -std=c++20 "
WASM_LDFLAGS = (
    " -nostartfiles -Wl,--no-entry -Wl,--lto-O3 -Wl,--strip-all "
    " -Wl,--strip-debug -Wl,--stack-first -Wl,--export-dynamic "
)

# Utilities we can use too
CLANG_FORMAT = ICPP_ROOT_COMPILER / "bin/clang-format"
CLANG_TIDY = ICPP_ROOT_COMPILER / "bin/clang-tidy"

########################################################################
# build-native
#
MOCKIC_C_FILES = (
    " ".join(
        [
            str(x)
            for x in list(ICPP_PATH.glob("ic/ic0mock/*.c"))
            + list(ICPP_PATH.glob("ic/candid/*.c"))
            + list(ICPP_PATH.glob("ic/icapi/*.c"))
            + list(ICPP_PATH.glob("ic/vendors/*.c"))
        ]
    )
    + " "
)
MOCKIC_CPP_FILES = (
    " ".join(
        [
            str(x)
            for x in list(ICPP_PATH.glob("ic/ic0mock/*.cpp"))
            + list(ICPP_PATH.glob("ic/candid/*.cpp"))
            + list(ICPP_PATH.glob("ic/icapi/*.cpp"))
            + list(ICPP_PATH.glob("ic/vendors/*.cpp"))
        ]
    )
    + " "
)
MOCKIC_HEADER_FILES = (
    " ".join(
        [
            str(x)
            for x in list(ICPP_PATH.glob("ic/ic0mock/*.h"))
            + list(ICPP_PATH.glob("ic/candid/*.h"))
            + list(ICPP_PATH.glob("ic/icapi/*.h"))
            + list(ICPP_PATH.glob("ic/vendors/*.h"))
            + list(ICPP_PATH.glob("ic/vendors/*.hpp"))
        ]
    )
    + " "
)

NATIVE_COMPILER = "Clang"
NATIVE_C = "clang"
NATIVE_CPP = "clang++"
NATIVE_CFLAGS = (
    f" -g  -I {ICPP_DIR}/ic/candid -I {ICPP_DIR}/ic/icapi -I {ICPP_DIR}/ic/ic0mock "
    f" -I {ICPP_DIR}/ic/vendors "
    f" -D ICPP_VERBOSE=0 "
)
NATIVE_CPPFLAGS = NATIVE_CFLAGS + " -std=c++20 "
NATIVE_LDFLAGS = " "
