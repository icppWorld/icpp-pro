#!/bin/bash

#####################################################################
# Git Bash script for QA Windows, mimics Makefile:                  #
# (-) called from either .git\hooks\pre-commit or windows-all-*.ps1 #
# (-) will run in git-bash                                          #
#####################################################################

# Get the Python interpreter and directories from the command-line arguments
PYTHON=$1
WHAT=$2  # "all-tests" or "all-static"

echo "$PYTHON interpreter:"
echo $PYTHON
$PYTHON --version

###########################################################################
# latest release of didc
VERSION_DIDC=$(curl --silent "https://api.github.com/repos/dfinity/candid/releases/latest" | grep -e '"tag_name"' | cut -c 16-25)
# version to install for clang
VERSION_CLANG=$(cat version_clang.txt)

###########################################################################
# Use some clang tools that come with wasi-sdk
WASI_SDK_COMPILER_VERSION=$($PYTHON -c "import sys; sys.path.append('src'); from src.icpp.version_wasi_sdk import __version__; print(__version__)")
WASI_SDK_COMPILER_ROOT=$($PYTHON -c "import sys; sys.path.append('src'); from src.icpp.config_default import WASI_SDK_COMPILER_ROOT; print(WASI_SDK_COMPILER_ROOT)")

CLANG_FORMAT="$WASI_SDK_COMPILER_ROOT/bin/clang-format"
CLANG_TIDY="$WASI_SDK_COMPILER_ROOT/bin/clang-tidy"

# Display the variables (optional)
echo "VERSION_DIDC: $VERSION_DIDC"
echo "VERSION_CLANG: $VERSION_CLANG"
echo "WASI_SDK_COMPILER_VERSION: $WASI_SDK_COMPILER_VERSION"
echo "WASI_SDK_COMPILER_ROOT: $WASI_SDK_COMPILER_ROOT"
echo "CLANG_FORMAT: $CLANG_FORMAT"
echo "CLANG_TIDY: $CLANG_TIDY"

###########################################################################
# Function to check the exit status of the last executed command
# (-) exit the script if not 0
check_exit_status() {
    if [ $? -ne 0 ]; then
        echo "ERROR."
        exit 1
    fi
}

###########################################################################
# cpp-format
CPP_AND_H_FILES=$(ls \
icpp-candid/src/icpp_candid/*/*.cpp icpp-candid/src/icpp_candid/*/*.h \
src/icpp/ic/*/*.cpp src/icpp/ic/*/*.h \
src/icpp/canisters/*/src/*.cpp src/icpp/canisters/*/src/*.h \
src/icpp/canisters/*/native/*.cpp src/icpp/canisters/*/native/*.h \
test/canisters/*/src/*.cpp test/canisters/*/src/*.h \
test/canisters/*/native/*.cpp test/canisters/*/native/*.h)
echo "------"
echo "cpp-format"
$CLANG_FORMAT --style=file --verbose -i $CPP_AND_H_FILES
check_exit_status

# Python files
PYTHON_DIRS="test src/icpp"

echo "------"
echo "python-format"
$PYTHON -m black $PYTHON_DIRS
check_exit_status

echo "------"
echo "python-lint"
$PYTHON -m pylint --jobs=0 --rcfile=.pylintrc $PYTHON_DIRS
check_exit_status

echo "------"
echo "python-type"
$PYTHON -m mypy --config-file .mypy.ini --show-column-numbers --strict $PYTHON_DIRS
check_exit_status

###########################################################################
# Exit if requested to only run "all-static"
if [ "$WHAT" == "all-static" ]; then
    exit 0
fi

###########################################################################
echo "------"
echo "all-canister-native"
$PYTHON -m scripts.all_canister_native
check_exit_status

echo "------"
echo "all-canister-deploy-local-pytest"
$PYTHON -m scripts.all_canister_deploy_local_pytest
check_exit_status