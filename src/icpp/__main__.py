"""__main__.py
The double underscores indicate that this file has a special meaning in Python.
When running a package (!) as a script with -m, 'python -m icpp', Python
executes the contents of the __main__.py file.

In other words, __main__.py acts as the entry point of our program and takes care of
the main flow, calling other parts as needed

reference: https://realpython.com/pypi-publish-python-package/
"""
import typer


# Instantiate the Typer app globally with app = typer.Typer().
# That way, we can decorate any function we want to call from the command line,
# using the @app.command() decorator.
app = typer.Typer()

# do some stuff with app
# pylint: disable = wrong-import-position unused-import
from icpp import callbacks
from icpp import commands_get
from icpp import commands_init
from icpp import commands_build_wasm
from icpp import commands_build_native
from icpp import commands_install_wasi_sdk


def main() -> None:
    """Entry point of program"""
    app(prog_name="icpp")


#
# Always start it up or debug as a module:
#  python -m icpp.__main__
#
main()
