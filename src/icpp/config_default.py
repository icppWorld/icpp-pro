"""
Set the default config variables
"""

import os
import sys
import platform
from pathlib import Path
import getpass
import icpp_candid  # pylint: disable = unused-import
from icpp import __version_wasi_sdk__, __version_rust__

# The OS we're running on - https://stackoverflow.com/a/1857/5480536

OS_SYSTEM = platform.system()
OS_PROCESSOR = platform.processor()

#######################################################################
# Paths for logs & to install dependencies (wasi-sdk, rust, ...)
ICPP_ROOT = Path.home() / ".icpp"
ICPP_LOGS = ICPP_ROOT / "logs"

# wasi-sdk compiler
WASI_SDK_ROOT = ICPP_ROOT / "wasi-sdk"
WASI_SDK_COMPILER_ROOT = WASI_SDK_ROOT / f"{__version_wasi_sdk__}"

# Rust dependencies
RUST_ROOT = ICPP_ROOT / "rust"
RUST_COMPILER_ROOT = RUST_ROOT / f"{__version_rust__}"
RUST_BIN = RUST_COMPILER_ROOT / "bin"
RUST_TARGET = RUST_COMPILER_ROOT / "target/wasm32-wasi/release"
CARGO = RUST_BIN / "cargo"
# CARGO_BINSTALL = RUST_BIN / "cargo-binstall"  # Not using this yet...
RUSTUP = RUST_BIN / "rustup"
RUSTC = RUST_BIN / "rustc"

WASI2IC = RUST_BIN / "wasi2ic"
IC_WASI_POLYFILL = RUST_TARGET / "libic_wasi_polyfill.a"

# Ensure that rust is installed in the correct folder
os.environ["CARGO_TARGET_DIR"] = str(  # pylint: disable = no-member
    RUST_COMPILER_ROOT / "target"
)
os.environ["CARGO_HOME"] = str(RUST_COMPILER_ROOT)  # pylint: disable = no-member
os.environ["RUSTUP_HOME"] = str(RUST_COMPILER_ROOT)  # pylint: disable = no-member

#######################################################################
USER = getpass.getuser()

ICPP_PATH = Path(sys.modules["icpp"].__path__[0])
ICPP_DIR = str(ICPP_PATH)

ICPP_CANDID_PATH = Path(sys.modules["icpp_candid"].__path__[0])
ICPP_CANDID_DIR = str(ICPP_CANDID_PATH)

########################################################################
# icpp.toml  (can be overwritten by --config)
#
ICPP_TOML_PATH = Path("icpp.toml")

########################################################################
# The greet canister created by `icpp init`
#
ICPP_INIT_PATH = ICPP_PATH / "canisters/greet"
LOCAL_INIT_PATH = Path("./greet")

########################################################################
# build-wasm
#
IC_C_FILES_LIST = [
    str(x)
    for x in list(ICPP_PATH.glob("ic/ic0/*.c"))
    + list(ICPP_PATH.glob("ic/wasi_sdk_traps/*.c"))
    + list(ICPP_PATH.glob("ic/canister/*.c"))
    + list(ICPP_PATH.glob("ic/icapi/*.c"))
    + list(ICPP_CANDID_PATH.glob("candid/*.c"))
]
IC_CPP_FILES_LIST = [
    str(x)
    for x in list(ICPP_PATH.glob("ic/ic0/*.cpp"))
    + list(ICPP_PATH.glob("ic/canister/*.cpp"))
    + list(ICPP_PATH.glob("ic/icapi/*.cpp"))
    + list(ICPP_PATH.glob("ic/pro/*.cpp"))
    + list(ICPP_CANDID_PATH.glob("candid/*.cpp"))
    + list(ICPP_CANDID_PATH.glob("hooks/*.cpp"))
    + list(ICPP_CANDID_PATH.glob("vendors/hash-library/*.cpp"))
]
IC_HEADER_FILES_LIST = [
    str(x)
    for x in list(ICPP_PATH.glob("ic/ic0/*.h"))
    + list(ICPP_PATH.glob("ic/wasi_sdk_traps/*.h"))
    + list(ICPP_PATH.glob("ic/canister/*.h"))
    + list(ICPP_PATH.glob("ic/icapi/*.h"))
    + list(ICPP_PATH.glob("ic/pro/*.h"))
    + list(ICPP_CANDID_PATH.glob("candid/*.h"))
    + list(ICPP_CANDID_PATH.glob("hooks/*.h"))
    + list(ICPP_CANDID_PATH.glob("vendors/hash-library/*.h"))
    + list(ICPP_CANDID_PATH.glob("vendors/cppcodec/*.hpp"))
    + list(ICPP_CANDID_PATH.glob("vendors/cppcodec/*/*.hpp"))
]

IC_C_FILES = " ".join(IC_C_FILES_LIST) + " "
IC_CPP_FILES = " ".join(IC_CPP_FILES_LIST) + " "
IC_HEADER_FILES = " ".join(IC_HEADER_FILES_LIST) + " "

SYSROOT = WASI_SDK_COMPILER_ROOT / "share/wasi-sysroot"

WASM_C_REQUIRED_FLAGS = (
    f" --target=wasm32-wasi --sysroot {SYSROOT} "
    f" -I {ICPP_DIR}/ic/canister -I {ICPP_DIR}/ic/icapi "
    f" -I {ICPP_DIR}/ic/pro -I {ICPP_DIR}/ic/ic0 "
    f" -I {ICPP_CANDID_PATH}/candid -I {ICPP_CANDID_PATH}/hooks "
    f" -I {ICPP_CANDID_PATH}/vendors "
)
WASM_CPP_REQUIRED_FLAGS = WASM_C_REQUIRED_FLAGS + " -std=c++20 "

WASM_C = WASI_SDK_COMPILER_ROOT / "bin/clang"
WASM_CPP = WASI_SDK_COMPILER_ROOT / "bin/clang++"
WASM_AR = WASI_SDK_COMPILER_ROOT / "bin/llvm-ar"
WASM_CFLAGS = (
    " -O3 -flto -fno-exceptions -fvisibility=hidden -D NDEBUG -D ICPP_VERBOSE=0 "
)
WASM_CPPFLAGS = WASM_CFLAGS
WASM_LDFLAGS = " -Wl,--lto-O3 -Wl,--strip-debug -Wl,--stack-first -Wl,--export-dynamic "
WASM_ARFLAGS = "qc"
WASM_AR_EXT = ".a"

# Utilities we can use too
CLANG_FORMAT = WASI_SDK_COMPILER_ROOT / "bin/clang-format"
CLANG_TIDY = WASI_SDK_COMPILER_ROOT / "bin/clang-tidy"
LLVM_OBJCOPY = WASI_SDK_COMPILER_ROOT / "bin/llvm-objcopy"

########################################################################
# build-native
#
MOCKIC_C_FILES_LIST = [
    str(x)
    for x in list(ICPP_PATH.glob("ic/ic0mock/*.c"))
    + list(ICPP_PATH.glob("ic/canister/*.c"))
    + list(ICPP_PATH.glob("ic/icapi/*.c"))
    + list(ICPP_CANDID_PATH.glob("candid/*.c"))
]

MOCKIC_CPP_FILES_LIST = [
    str(x)
    for x in list(ICPP_PATH.glob("ic/ic0mock/*.cpp"))
    + list(ICPP_PATH.glob("ic/canister/*.cpp"))
    + list(ICPP_PATH.glob("ic/icapi/*.cpp"))
    + list(ICPP_PATH.glob("ic/pro/*.cpp"))
    + list(ICPP_CANDID_PATH.glob("candid/*.cpp"))
    + list(ICPP_CANDID_PATH.glob("hooks/*.cpp"))
    + list(ICPP_CANDID_PATH.glob("vendors/hash-library/*.cpp"))
]
MOCKIC_HEADER_FILES_LIST = [
    str(x)
    for x in list(ICPP_PATH.glob("ic/ic0mock/*.h"))
    + list(ICPP_PATH.glob("ic/canister/*.h"))
    + list(ICPP_PATH.glob("ic/icapi/*.h"))
    + list(ICPP_PATH.glob("ic/pro/*.h"))
    + list(ICPP_CANDID_PATH.glob("candid/*.h"))
    + list(ICPP_CANDID_PATH.glob("hooks/*.h"))
    + list(ICPP_CANDID_PATH.glob("vendors/hash-library/*.h"))
    + list(ICPP_CANDID_PATH.glob("vendors/cppcodec/*.hpp"))
    + list(ICPP_CANDID_PATH.glob("vendors/cppcodec/*/*.hpp"))
]

MOCKIC_C_FILES = " ".join(MOCKIC_C_FILES_LIST) + " "
MOCKIC_CPP_FILES = " ".join(MOCKIC_CPP_FILES_LIST) + " "
MOCKIC_HEADER_FILES = " ".join(MOCKIC_HEADER_FILES_LIST) + " "

NATIVE_COMPILER = "Clang"
NATIVE_C = "clang"
NATIVE_CPP = "clang++"
NATIVE_AR = "ar"

NATIVE_C_REQUIRED_FLAGS = (
    f" -I {ICPP_DIR}/ic/canister -I {ICPP_DIR}/ic/icapi "
    f" -I {ICPP_DIR}/ic/pro -I {ICPP_DIR}/ic/ic0mock "
    f" -I {ICPP_CANDID_PATH}/candid -I {ICPP_CANDID_PATH}/hooks "
    f" -I {ICPP_CANDID_PATH}/vendors "
)
NATIVE_CPP_REQUIRED_FLAGS = NATIVE_C_REQUIRED_FLAGS + " -std=c++20 "

NATIVE_CFLAGS = " -g -D ICPP_VERBOSE=0 "
NATIVE_CPPFLAGS = NATIVE_CFLAGS
NATIVE_LDFLAGS = " "

NATIVE_ARFLAGS = "qc"
NATIVE_AR_EXT = ".a"
