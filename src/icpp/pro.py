"""
Handles licensing of icpp_pro

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
        typer.echo("ERROR: this command requires icpp_pro")
        typer.echo("For licensing, contact us at icpp@icpp.world")
        sys.exit(1)


def is_pro() -> bool:
    """Returns true if running a licensed version of icpp_pro."""
    return True
