"""
Handles licensing of icpp-pro

********************************************************************************
* IT IS NOT ALLOWED TO MODIFY THIS FILE OR ANY FILE THAT CALLS THESE FUNCTIONS *
********************************************************************************

"""
import sys
import typer


def exit_if_not_pro() -> None:
    """Exit if not running a licensed icpp-pro."""
    if not is_pro():
        typer.echo("--")
        typer.echo("ERROR: this command requires icpp-pro")
        typer.echo(" ")
        typer.echo("You can just upgrade with: `pip install icpp-pro`")
        typer.echo("But please be a sport, and donate a little. Thank YOU!")
        typer.echo(
            "For details, see: https://docs.icpp.world/index.html#donations-welcome"
        )
        sys.exit(1)


def is_pro() -> bool:
    """Returns true if running a licensed version of icpp-pro."""
    return True
