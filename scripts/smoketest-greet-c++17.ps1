###########################################################################
# This is a Windows PowerShell script for smoketesting the greet canister #
###########################################################################
Write-Host " "
Write-Host "--------------------------------------------------"
Write-Host "Stopping the local network in wsl"
wsl --% . ~/.local/share/dfx/env; dfx stop

Write-Host " "
Write-Host "--------------------------------------------------"
Write-Host "Starting the local network in wsl as a PowerShell background job"
$jobName = "smoketest"

# Stop the job if it already exist
# Get a list of all the background jobs with a specific name
$jobs = Get-Job -Name $jobName -ErrorAction SilentlyContinue

# Stop all the jobs
$jobs | Remove-Job

# Start the local network with redirecting error messages to output stream
$job = Start-Job -ScriptBlock { wsl bash -c ". ~/.local/share/dfx/env; dfx start --clean 2>&1" } -Name $jobName

# Display the details of the job
$job | Format-Table

# Wait for 30 seconds
Write-Host " "
Write-Host "Waiting for 30 seconds to allow the local network to start..."
Start-Sleep -Seconds 30

# Get the output of the job
$output = Receive-Job -Job $job
Write-Host $output -ForegroundColor Green

#######################################################################
Write-Host " "
Write-Host "--------------------------------------------------"
Write-Host "Building the libraries for c++17"
icpp build-library --config icpp-c++17.toml
icpp build-library --config icpp-c++17.toml libhello

#######################################################################
Write-Host " "
Write-Host "--------------------------------------------------"
Write-Host "Building the wasm with wasi-sdk"
# icpp build-wasm --config icpp-c++17.toml --to-compile all
icpp build-wasm --config icpp-c++17.toml --to-compile mine-no-lib

#######################################################################
Write-Host " "
Write-Host "--------------------------------------------------"
Write-Host "Deploying the wasm to a canister on the local network"
wsl --% . ~/.local/share/dfx/env; dfx deploy

#######################################################################
Write-Host " "
Write-Host "--------------------------------------------------"
Write-Host "Running the full smoketests with pytest"
pytest --network=local

# Capture the exit code of the pytest command
$pytestExitCode = $LASTEXITCODE

#######################################################################
Write-Host " "
Write-Host "--------------------------------------------------"
Write-Host "Stopping the local network in wsl"
wsl --% . ~/.local/share/dfx/env; dfx stop

#######################################################################
# Check if the pytestExitCode is non-zero
if ($pytestExitCode -ne 0) {
    # Print the exit code
    Write-Output "Pytest had a failure - Exit code: $pytestExitCode"

    # Exit the script with the same exit code as pytest
    exit $pytestExitCode
}