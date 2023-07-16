#######################################################################
# This is a Windows PowerShell script
#
# (-) Install icpp-pro or icpp-free in a python environment on windows
# (-) Install dfx  in wsl
# (-) In a Windows PowerShell (*):
#
#     .\demo.ps1
#
# (*) The Miniconda Powershell is highly recommended
#
#######################################################################
Write-Host " "
Write-Host "--------------------------------------------------"
Write-Host "Stopping the local network in wsl"
wsl dfx stop

Write-Host " "
Write-Host "--------------------------------------------------"
Write-Host "Starting the local network in wsl as a PowerShell background job"
$jobName = "canister_1"

# Stop the job if it already exist
# Get a list of all the background jobs with a specific name
$jobs = Get-Job -Name $jobName -ErrorAction SilentlyContinue

# Stop all the jobs
$jobs | Remove-Job

# Start the local network with redirecting error messages to output stream
$job = Start-Job -ScriptBlock { wsl dfx start --clean 2>&1 } -Name $jobName

# Display the details of the job
$job | Format-Table

# Wait for 10 seconds
Write-Host " "
Write-Host "Waiting for 10 seconds to allow the local network to start..."
Start-Sleep -Seconds 10

# Get the output of the job
$output = Receive-Job -Job $job
Write-Host $output -ForegroundColor Green

#######################################################################
Write-Host " "
Write-Host "--------------------------------------------------"
Write-Host "Building the wasm with wasi-sdk"
icpp build-wasm --to-compile all
# icpp build-wasm --to-compile mine

#######################################################################
Write-Host " "
Write-Host "--------------------------------------------------"
Write-Host "Deploying the wasm to a canister on the local network"
wsl --% dfx deploy

#######################################################################
Write-Host " "
Write-Host "--------------------------------------------------"
Write-Host "Running some manual tests with dfx"
wsl --% dfx canister call my_canister test_candid
wsl --% dfx canister call my_canister test_ic_api
wsl --% dfx canister call my_canister roundtrip_bool_true '(true)'
wsl --% dfx canister call my_canister roundtrip_variant_ok '(variant { Ok })'
wsl --% dfx canister call my_canister roundtrip_variant_err '(variant { Err = "Error" : text})'

#######################################################################
Write-Host " "
Write-Host "--------------------------------------------------"
Write-Host "Running the full smoketests with pytest"
pytest --network=local

#######################################################################
Write-Host " "
Write-Host "--------------------------------------------------"
Write-Host "Stopping the local network in wsl"
wsl dfx stop

#######################################################################
Write-Host " "
Write-Host "--------------------------------------------------"
Write-Host "Building the Windows native debug executable with clang++"
icpp build-native --to-compile all
# icpp build-native --to-compile mine

#######################################################################
Write-Host " "
Write-Host "--------------------------------------------------"
Write-Host "Running the Windows native debug executable"
.\build-native\mockic.exe