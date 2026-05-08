"""__main__.py
The double underscores indicate that this file has a special meaning in Python.
When running a package (!) as a script with -m, 'python -m icpp', Python
executes the contents of the __main__.py file.

In other words, __main__.py acts as the entry point of our program and takes care of
the main flow, calling other parts as needed

reference: https://realpython.com/pypi-publish-python-package/
"""

import os
import sys
import typer

# Instantiate the Typer app globally with app = typer.Typer().
# That way, we can decorate any function we want to call from the command line,
# using the @app.command() decorator.
app = typer.Typer()

# do some stuff with app
# pylint: disable = wrong-import-position unused-import
from icpp import options_main
from icpp import commands_get
from icpp import commands_init
from icpp import commands_build_wasm
from icpp import commands_build_native
from icpp import commands_build_library
from icpp import commands_build_library_native
from icpp import commands_install_wasi_sdk
from icpp import commands_install_rust


def main() -> None:
    """Entry point of program"""

    # Add this to find user's extensions in sub-folders
    sys.path.insert(1, os.getcwd())  # pylint: disable=no-member

    app(prog_name="icpp")


# The console-script `icpp` (created by [project.scripts]) imports and
# calls main() explicitly. The guard prevents main() from firing on plain
# imports (e.g. `from icpp import commands_build_wasm`), which would
# otherwise invoke the CLI with the host process's sys.argv.
if __name__ == "__main__":
    main()
