# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

icpp-pro is a C++ Canister Development Kit (CDK) for the Internet Computer. It's a Python CLI tool (`icpp`) that compiles C++ smart contracts to WebAssembly for deployment on the IC blockchain, with a native build mode for local debugging using a Mock IC.

The source code is open source:
- https://github.com/icppWorld/icpp-pro
- https://github.com/icppWorld/icpp-candid

**Key dependency:** `icpp-candid` is cloned as a nested repo inside `icpp-pro/icpp-candid/` and installed separately.

## Development Setup

```bash
# Conda environment
conda create --name icpp-pro python=3.11
conda activate icpp-pro

# Install icpp-candid + icpp-pro in editable mode with dev deps
make install-python

# Install toolchains
icpp install-wasi-sdk
icpp install-rust
make install-dfx
```

## Common Commands

| Command                                  | Purpose                                              |
|------------------------------------------|------------------------------------------------------|
| `make all-tests`                         | Run everything: static checks + native builds + deploy + pytest |
| `make all-static`                        | All static checks (cpp-format, cpp-lint, python-format, python-lint, python-type) |
| `make all-canister-native`               | Build all canisters as native executables             |
| `make all-canister-deploy-local-pytest`  | Deploy canisters to local dfx network and run pytest  |
| `make python-format`                     | Format Python with black                              |
| `make python-lint`                       | Lint Python with pylint                               |
| `make python-type`                       | Type check Python with mypy (strict mode)             |
| `make cpp-format`                        | Format C++ with clang-format from wasi-sdk            |
| `make install-python`                    | Reinstall icpp-candid + icpp-pro in editable mode     |
| `make pypi-build`                        | Build distribution packages                           |

### Running a Single Canister Test

```bash
# Native build (for debugging) - from a canister directory:
cd test/canisters/canister_1
icpp build-native
./build-native/mockic.exe

# WASM build + deploy + pytest against local network:
cd test/canisters/canister_1
icpp build-wasm
dfx start --clean --background
dfx deploy
pytest --network=local
```

## Architecture

### Source Layout (src-layout)

- `src/icpp/` - Main Python package
  - `__main__.py` - CLI entry point using Typer (`icpp` command)
  - `commands_build_wasm.py` / `commands_build_native.py` - Core build commands
  - `commands_build_library.py` / `commands_build_library_native.py` - Library builds
  - `commands_init.py` - `icpp init` scaffolding
  - `commands_install_wasi_sdk.py` / `commands_install_rust.py` - Toolchain installers
  - `icpp_toml.py` - Parser for `icpp.toml` canister configuration files
  - `config_default.py` - Global defaults (paths, compiler flags, tool versions)
  - `smoketest.py` - Utilities for canister API testing (`call_canister_api`)
  - `conftest_base.py` - Shared pytest fixtures (network, identity, principal)
  - `run_shell_cmd.py` / `run_dfx_cmd.py` - Command execution helpers
  - `decorators.py` - e.g. `@requires_wasi_sdk()`
  - `version*.py` - Pinned versions for wasi-sdk, rust, wasi2ic, ic-wasi-polyfill
- `src/icpp/ic/` - C++ headers and source for IC bindings
  - `ic0/` - Low-level IC0 API (C bindings)
  - `ic0mock/` - Mock IC0 for native debugging
  - `canister/` - Canister lifecycle hooks
  - `icapi/` - IC API C++ wrappers
  - `pro/` - Pro features
- `src/icpp/canisters/greet/` - Template canister (used by `icpp init`)
- `test/canisters/` - Integration test canisters (canister_1, canister_flags, canister_http, etc.)
- `scripts/` - CI orchestration (all_canister_native.py, all_canister_deploy_local_pytest.py)
- `icpp-candid/` - Nested dependency repo (C++ Candid library, separate PyPI package)

### Build Modes

Every canister supports two build targets configured in `icpp.toml`:
- **WASM** (`icpp build-wasm`): Production builds using wasi-sdk compiler, outputs `.wasm`
- **Native** (`icpp build-native`): Debug builds using system clang, outputs `mockic.exe` with Mock IC

### Configuration Files

- `icpp.toml` - Per-canister build configuration (source files, include dirs, compiler flags)
- `dfx.json` - Internet Computer SDK canister definitions
- `.clang-format` - C++ formatting rules (from wasi-sdk)
- `.pylintrc` - Pylint configuration
- `.mypy.ini` - Mypy strict mode configuration

### External Toolchains (installed to `~/.icpp/`)

- wasi-sdk (WASM compiler)
- Rust (for wasi2ic converter)
- dfx (Internet Computer SDK, installed to `~/bin/`)

## Code Quality

- **Python formatting:** black 23.12.1
- **Python linting:** pylint 2.13.9
- **Python type checking:** mypy 1.8.0 (strict mode)
- **C++ formatting:** clang-format from wasi-sdk
- **Pre-commit hook:** `cp pre-commit-mac .git/hooks/pre-commit && chmod +x .git/hooks/pre-commit`

## Testing

Tests run in two modes:
1. **Native:** Build with `icpp build-native`, run `mockic.exe` directly
2. **Deployed:** Build WASM, deploy to local dfx network, run pytest with `--network=local`

Pytest fixtures from `conftest_base.py` provide: `network`, `identity`, `principal`, `identity_anonymous`, `identity_default`. Test conftest files import these with `from icpp.conftest_base import *`.

## Version Management

Version is single-sourced in `src/icpp/version.py`. Current: 5.3.0. Release scripts modify this file automatically.
