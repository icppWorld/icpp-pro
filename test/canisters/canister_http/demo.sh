#!/bin/sh

#######################################################################
# This is a Linux & Mac shell script
#
# (-) Install icpp-pro or icpp-free in a python environment
# (-) Install dfx
# (-) In a terminal:
#
#     ./demo.sh
#
#######################################################################
echo " "
echo "--------------------------------------------------"
echo "Stopping the local network"
dfx stop

echo " "
echo "--------------------------------------------------"
echo "Starting the local network as a background process"
dfx start --clean --background

#######################################################################
echo "--------------------------------------------------"
echo "Building the wasm with wasi-sdk"
icpp build-wasm --to-compile all
# icpp build-wasm --to-compile mine

#######################################################################
echo " "
echo "--------------------------------------------------"
echo "Deploying the wasm to a canister on the local network"
dfx deploy

#######################################################################
echo " "
echo "--------------------------------------------------"
echo "Running the full smoketests with pytest"
pytest --network=local

#######################################################################
echo " "
echo "--------------------------------------------------"
echo "Calling the deployed canister with curl, skipping the asserts"
curl -X GET -H "skip-asserts: yes" -H "Content-Type: application/json" -d '{"key1":"value1", "key2":"value2"}' http://localhost:$(dfx info webserver-port)/?canisterId=$(dfx canister id my_canister)

#######################################################################
echo "--------------------------------------------------"
echo "Stopping the local network"
dfx stop

#######################################################################
echo " "
echo "--------------------------------------------------"
echo "Building the OS native debug executable with clang++"
icpp build-native --to-compile all
# icpp build-native --to-compile mine

#######################################################################
echo " "
echo "--------------------------------------------------"
echo "Running the OS native debug executable"
./build-native/mockic.exe