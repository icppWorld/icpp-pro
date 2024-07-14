# Get the path to the current conda environment's python interpreter
$condaInfo = conda info --json | ConvertFrom-Json
$activeEnvPath = $condaInfo.envs | Where-Object { $_ -eq $condaInfo.default_prefix }

if (-not $activeEnvPath) {
    Write-Error "Active Conda environment not found."
    exit 1
}

$pythonInterpreter = Join-Path $activeEnvPath "python.exe"

# Define the path to the Git Bash executable
$gitBashPath = "C:\Program Files\Git\bin\bash.exe"

# ---------------------------------------------------------
# Run the tests

# Define the path to the all-test bash script
$winAllTestPath = ".\windows-all-tests.sh"

# Execute the bash script using Git Bash
& $gitBashPath $winAllTestPath $pythonInterpreter "all-tests"

# Capture the exit code from Git Bash
$exitCodeAllStatic = $LASTEXITCODE

# Exit PowerShell with the same exit code only if it's not 0
if ($exitCodeAllStatic -ne 0) {
    exit $exitCodeAllStatic
}