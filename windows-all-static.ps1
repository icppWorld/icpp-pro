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

# Define the path to the pre-commit script
$winAllStaticPath = ".\win-all-static.sh"

# Execute the pre-commit script using Git Bash, passing the variables as arguments
& $gitBashPath $winAllStaticPath $pythonInterpreter

# Capture the exit code from Git Bash
$exitCode = $LASTEXITCODE

# Exit PowerShell with the same exit code
exit $exitCode
