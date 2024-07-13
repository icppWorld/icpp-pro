#!/bin/bash

PYTHON_DIRS="test src/icpp"

# Get the Python interpreter and directories from the command-line arguments
PYTHON_INTERPRETER=$1

echo "Executing win-all-static.sh, with python interpreter:"
echo $PYTHON_INTERPRETER
$PYTHON_INTERPRETER --version

# Function to check the exit status of the last executed command
check_exit_status() {
    if [ $? -ne 0 ]; then
        echo "ERROR."
        exit 1
    fi
}

# Apply all static auto-formatting & perform the static checks
echo "------"
echo "$PYTHON_INTERPRETER -m black $PYTHON_DIRS"
$PYTHON_INTERPRETER -m black $PYTHON_DIRS
check_exit_status

echo "------"
echo "$PYTHON_INTERPRETER -m pylint --jobs=0 --rcfile=.pylintrc $PYTHON_DIRS"
$PYTHON_INTERPRETER -m pylint --jobs=0 --rcfile=.pylintrc $PYTHON_DIRS
check_exit_status

echo "------"
echo "$PYTHON_INTERPRETER -m mypy --config-file .mypy.ini --show-column-numbers --strict $PYTHON_DIRS"
$PYTHON_INTERPRETER -m mypy --config-file .mypy.ini --show-column-numbers --strict $PYTHON_DIRS
check_exit_status