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
icpp build-wasm --to-compile all
# icpp build-wasm --to-compile mine-no-lib

#######################################################################
echo " "
echo "--------------------------------------------------"
echo "Deploying the wasm to a canister on the local network"
icp deploy --environment local --yes

#######################################################################
echo " "
echo "--------------------------------------------------"
echo "Running some manual tests with icp"
icp canister call greet greet_0 '()' --environment local
icp canister call greet greet_0_static_lib '()' --environment local
icp canister call greet greet_1 '()' --environment local
icp canister call greet greet_2 '("C++ Developer")' --environment local
icp canister call greet greet_3 '(record { "icpp version" = 1 : int; OS = "Linux" : text })' --environment local
icp canister call greet greet_4 '(record { 6 = 42 : int; 9 = 43 : int }, record { 7 = 44 : int; 10 = 45 : int })' --environment local
icp canister call greet greet_json '("{\"name\": \"AJ\"}")' --environment local
icp canister call greet greet_log_file '()' --environment local

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
icpp build-native --to-compile all
# icpp build-native --to-compile mine-no-lib

#######################################################################
echo " "
echo "--------------------------------------------------"
echo "Running the OS native debug executable"
./build-native/mockic.exe