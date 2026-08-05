#!/bin/sh

#######################################################################
# This is a Linux & Mac shell script
#
# (-) Install icpp-pro in a python environment
# (-) Install icp-cli: npm install -g @icp-sdk/icp-cli
# (-) In a terminal:
#
#     ./demo.sh
#
#######################################################################
# The identity we deploy with and run the tests as. It is named explicitly and
# passed to every icp command, so the machine-wide active identity
# (`icp identity default`) is never read and never changed - running this demo
# cannot disturb the identity you use for mainnet work.
ICPP_PRO_TEST_IDENTITY=${ICPP_PRO_TEST_IDENTITY:-greet-testing}
export ICPP_PRO_TEST_IDENTITY
# `icp identity principal` exits non-zero for an unknown name, which is an exact
# check - unlike grepping `icp identity list`.
icp identity principal --identity "$ICPP_PRO_TEST_IDENTITY" >/dev/null 2>&1 || \
  icp identity new "$ICPP_PRO_TEST_IDENTITY" --storage plaintext

#######################################################################
echo " "
echo "--------------------------------------------------"
echo "Stopping the local network"
icp network stop

echo " "
echo "--------------------------------------------------"
echo "Starting a clean local network as a background process"
# icp-cli has no `--clean` flag: a managed network keeps its state in .icp/cache
rm -rf .icp/cache
icp network start --background

#######################################################################
echo "--------------------------------------------------"
echo "Building the wasm with wasi-sdk"
icpp build-wasm --config icpp-c++17.toml --to-compile all
# icpp build-wasm --config icpp-c++17.toml --to-compile mine-no-lib

#######################################################################
echo " "
echo "--------------------------------------------------"
echo "Deploying the wasm to a canister on the local network"
icp deploy --environment local --yes --identity "$ICPP_PRO_TEST_IDENTITY"

#######################################################################
echo " "
echo "--------------------------------------------------"
echo "Running some manual tests with icp"
icp canister call greet greet_0 '()' --environment local --identity "$ICPP_PRO_TEST_IDENTITY"
icp canister call greet greet_0_static_lib '()' --environment local --identity "$ICPP_PRO_TEST_IDENTITY"
icp canister call greet greet_1 '()' --environment local --identity "$ICPP_PRO_TEST_IDENTITY"
icp canister call greet greet_2 '("C++ Developer")' --environment local --identity "$ICPP_PRO_TEST_IDENTITY"
icp canister call greet greet_3 '(record { "icpp version" = 1 : int; OS = "Linux" : text })' --environment local --identity "$ICPP_PRO_TEST_IDENTITY"
icp canister call greet greet_4 '(record { 6 = 42 : int; 9 = 43 : int }, record { 7 = 44 : int; 10 = 45 : int })' --environment local --identity "$ICPP_PRO_TEST_IDENTITY"
icp canister call greet greet_json '("{\"name\": \"AJ\"}")' --environment local --identity "$ICPP_PRO_TEST_IDENTITY"
icp canister call greet greet_log_file '()' --environment local --identity "$ICPP_PRO_TEST_IDENTITY"

#######################################################################
echo " "
echo "--------------------------------------------------"
echo "Running the full smoketests with pytest"
pytest --network=local

#######################################################################
echo "--------------------------------------------------"
echo "Stopping the local network"
icp network stop

#######################################################################
echo " "
echo "--------------------------------------------------"
echo "Building the OS native debug executable with clang++"
icpp build-native --config icpp-c++17.toml --to-compile all
# icpp build-native --config icpp-c++17.toml --to-compile mine-no-lib

#######################################################################
echo " "
echo "--------------------------------------------------"
echo "Running the OS native debug executable"
./build-native/mockic.exe