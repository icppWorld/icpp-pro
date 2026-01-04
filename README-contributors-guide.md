# Contributors Guide

If you are a developer of `icpp-pro` itself, this guide is for you.

- `icpp-candid` is a Python package, released from the public GitHub repo: https://github.com/icppWorld/icpp-candid
- `icpp-pro` is a Python packages, released from the private GitHub repo: https://github.com/icppWorld/icpp-pro
- You can work on a Mac, Ubuntu or Windows WSL Ubuntu

See the release-guide files in this repo for details on the release process for each package.

See the [installation instructions](https://docs.icpp.world) for details on the installation process for our users.

## Setup

```bash
# clone the icpp-pro & icpp-candid repo as nested
git clone https://github.com/icppWorld/icpp-pro
cd icpp-pro
git clone https://github.com/icppWorld/icpp-candid

# When developing icpp-pro AND icpp_llm, llama_cpp_canister
# clone those repos as siblings of icpp-pro
```

We use [MiniConda](https://docs.anaconda.com/miniconda/install/#quick-command-line-install) for dependency isolation.

```bash
# create & activate a MiniConda environment
# (-) When developing just icpp-pro or icpp-candid
conda create --name icpp-pro python=3.11
conda activate icpp-pro
# (-) When developing icpp-pro, icpp-candid & icpp-demos
conda create --name icpp-pro-w-demos python=3.11
conda activate icpp-pro-w-demos
# (-) When developing icpp-pro, icpp-candid & icpp-llm
conda create --name icpp-pro-w-icpp-llm python=3.11
conda activate icpp-pro-w-icpp-llm
# (-) When developing icpp-pro, icpp-candid & llama_cpp_canister
conda create --name icpp_pro_w_llama_cpp_canister python=3.11
conda activate icpp_pro_w_llama_cpp_canister


# Clone the following repos using this folder structure:
# `./icpp-pro`
# `./icpp-pro/icpp-candid`
# `./icpp-demos`
# `./icpp_llm`
# `./llama_cpp_canister`
# `./llama_cpp_canister/src/llama_cpp_onicai_fork`

# Start fresh
pip cache purge

# Install `icpp-candid` & `icpp-pro`/`icpp-free` & icpp-demos in editable mode with development dependencies
# Notes:
# - First time you run these make commands, you will get this error that you can ignore:
#    Traceback (most recent call last):
#      File "<string>", line 1, in <module>
#      File "/Users/arjaan/icppWorld/repos/icpp-pro/src/icpp/config_default.py", line 10, in <module>
#        import icpp_candid  # pylint: disable = unused-import
#        ^^^^^^^^^^^^^^^^^^
#    ModuleNotFoundError: No module named 'icpp_candid'
#
cd icpp-pro
# (-) When developing just icpp-pro or icpp-candid
make install-python
# (-) When developing icpp-pro, icpp-candid & icpp-demos
make install-python-w-demos
# (-) When developing icpp-pro, icpp-candid & icpp-llm
make install-python-w-icpp-llm
# (-) When developing icpp-pro, icpp-candid & llama_cpp_canister
make install-python-w-llama_cpp_canister

# Verify all packages are correctly installed in editable mode
% pip list
icpp-candid        4.2.2       /Users/arjaan/icppWorld/repos/icpp-pro/icpp-candid
icpp-pro           4.2.2       /Users/arjaan/icppWorld/repos/icpp-pro

# Verify it works
% icpp --version
icpp-pro         version: 4.2.2
wasi-sdk         version: wasi-sdk-22.0
wasi2ic          version: c0f5063e734f8365f1946baf2845d8322cc9bfec
ic-wasi-polyfill version: b3ef005140e7eebf7d0b5471ccc3a6d4cbec4ee5
rust             version: 1.79.0

# Activate command completion
icpp --install-completion
```

### Install wasi-sdk & rust

For this, you can use icpp itself:

```bash
conda activate icpp-pro

icpp install-wasi-sdk
icpp install-rust
```

### Install dfx

```bash
make install-dfx

# start a new shell

# verify it works
dfx --version
```



### Install `didc`

A handy command line utility to test out [candid](https://github.com/dfinity/candid) generation:

```bash
# Make sure wget is installed
# on Mac, we use Homebrew
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
brew install wget

# Then install didc
make install-didc

# verify it works
didc --version
```

# pre-commit

The `icpp-pro` repository contains these files to easily set up pre-commit:

```
# Mac
cp pre-commit-mac .git/hooks/pre-commit
# Ubuntu
cp pre-commit-ubuntu .git/hooks/pre-commit

chmod +x .git/hooks/pre-commit

# test it works by running:
.git/hooks/pre-commit
```

The files look like this:
```bash
# Mac
#!/bin/bash

# Apply all static auto-formatting & perform the static checks
export PATH="/opt/miniconda3/envs/icpp-pro/bin:$PATH"
/usr/bin/make all-static

# ---------------
# Ubuntu
#!/bin/bash

# Apply all static auto-formatting & perform the static checks
export PATH="$HOME/miniconda3/envs/icpp-pro/bin:$PATH"
/usr/bin/make all-static
```

## Verify your setup

You can verify that your setup is complete by running:

```bash
make all-tests
```

## Development in VS Code

### Configure VS Code

Install these extensions:
- C/C++ from Microsoft
- C/C++ Extension Pack by Microsoft
- Hex Editor by Microsoft
- CodeLLDB by Vadim Chugonov
- Even Better TOML by `tamasfe`
- On Windows: WSL Ubuntu extension

Configure clang-format:
- Open the Settings editor, with the following VS Code menu command:
  - On Windows/Linux - File > Preferences > Settings
  - On macOS - Code > Settings > Settings
- Search for `clang-format`
  - C_Cpp: Clang_format_path: `~/.icpp/wasi-sdk/wasi-sdk-22.0/bin/clang-format`
    - (Note that the embedded clang-format works sligtly different...)
  - C_Cpp: Clang_format_style: file
    - (Note that this will use .clang-format in the project root folder)
- Search for `Editor:Format On Save` and turn it on.


- For C++ debugging in VS Code, with the C/C++ extensions installed, put this file in `.vscode/launch.json`:

  **Ubuntu**
  ```json
  {
    // Use IntelliSense to learn about possible attributes.
    // Hover to view descriptions of existing attributes.
    // For more information, visit: https://go.microsoft.com/fwlink/?linkid=830387
    "version": "0.2.0",
    "configurations": [
        {
            "type": "lldb",
            "request": "launch",
            "name": "Debug",
            // "program": "${workspaceFolder}/src/icpp/canisters/greet/build-native/mockic.exe",
            "program": "${workspaceFolder}/test/canisters/canister_1/build-native/mockic.exe",
            // "program": "${workspaceFolder}/test/canisters/canister_flags/build-native/mockic.exe",
            // "program": "${workspaceFolder}/test/canisters/canister_http/build-native/mockic.exe",
            "args": [],
            "cwd": "${workspaceFolder}"
        }
    ]
  }
  ```

  **Mac**
  ```json
  {
    // Use IntelliSense to learn about possible attributes.
    // Hover to view descriptions of existing attributes.
    // For more information, visit: https://go.microsoft.com/fwlink/?linkid=830387
    "version": "0.2.0",
    "configurations": [
        {
            "type": "cppdbg",
            "MIMode": "lldb",
            "request": "launch",
            "name": "Debug",
            // "program": "${workspaceFolder}/src/icpp/canisters/greet/build-native/mockic.exe",
            // "program": "${workspaceFolder}/test/canisters/canister_1/build-native/mockic.exe",
            // "program": "${workspaceFolder}/test/canisters/canister_flags/build-native/mockic.exe",
            "program": "${workspaceFolder}/test/canisters/canister_http/build-native/mockic.exe",
            "args": [],
            "cwd": "${workspaceFolder}"
        }
    ]
  }
  ```

  And then set breakpoints & click on the Debug icon.

- For C++ Intellisense, put this file in `.vscode/c_cpp_properties.json`

  **Ubuntu**
  ```json
  {
      "configurations": [
          {
              "name": "Linux",
              "includePath": [
                  "${workspaceFolder}/**",
                  "${workspaceFolder}/src/icpp/ic/**"
              ],
              "defines": [],
              "compilerPath": "/usr/bin/clang++",
              "cStandard": "c17",
              "cppStandard": "c++20",
              "intelliSenseMode": "linux-clang-x64"
          }
      ],
      "version": 4
  }
  ```

  **Mac**
  ```json
  {
    "configurations": [
        {
            "name": "macOS",
            "includePath": [
                "${workspaceFolder}/**",
                "${workspaceFolder}/src/icpp/ic/**"
            ],
            "defines": [],
            "compilerPath": "/usr/bin/clang++",
            "cStandard": "c17",
            "cppStandard": "c++20",
            "intelliSenseMode": "macos-clang-x64"
        }
    ],
    "version": 4
  }
  ```


## The tests

You can find test code in the folder `test/canister/canister_1`.

All tests are run automatically in two ways:
- Compiled into a native executable, with the Mock IC.
- Compiled to wasm, deployed to a local network, and run within the deployed canister, using API calls orchestrated with pytest.

### Native with Mock IC

This is nice for debugging. Just do the following:

```bash
# from the folder: test/canister/canister_1

# build the native executable
icpp build-native
```

Now, as the command output explains, you can:
- Run the `mockic.exe`
- Debug it in VS Code + CodeLLDB