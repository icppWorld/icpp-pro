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
# icpp build-wasm --to-compile mine

#######################################################################
echo " "
echo "--------------------------------------------------"
echo "Deploying the wasm to a canister on the local network"
icp deploy --environment local --yes

#######################################################################
echo " "
echo "--------------------------------------------------"
echo "Running the full smoketests with pytest"
pytest --network=local

#######################################################################
echo " "
echo "--------------------------------------------------"
echo "Calling the deployed canister with curl"
CANISTER_ID=$(icp canister status my_canister --environment local --json | python -c "import json,sys; print(json.load(sys.stdin)['id'])")
GATEWAY_PORT=$(icp network status --environment local --json | python -c "import json,sys; print(json.load(sys.stdin)['gateway_url'].rstrip('/').rsplit(':',1)[-1])")
curl -X GET http://$CANISTER_ID.raw.localhost:$GATEWAY_PORT/counter

#######################################################################
echo " "
echo "--------------------------------------------------"
echo "Stopping the local network"
icp network stop

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