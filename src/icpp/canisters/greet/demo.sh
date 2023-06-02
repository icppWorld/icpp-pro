#!/bin/sh

echo "------------------------------------------"
echo "Starting a clean local network"
dfx stop
dfx start --clean --background

echo "------------------------------------------"
echo "Building & deploying the canister"
dfx deploy

echo "------------------------------------------"
echo "Running some tests with dfx"
dfx canister call greet greet_0
dfx canister call greet greet_1
dfx canister call greet greet_2 '("C++ Developer")'
dfx canister call greet greet_3 '(record { "icpp version" = 1 : int; OS = "Linux" : text })'
dfx canister call greet greet_4 '(record { 6 = 42 : int; 9 = 43 : int }, record { 7 = 44 : int; 10 = 45 : int })'

echo "------------------------------------------"
echo "Running the smoketests (requires icpp-pro)"
pytest --network=local