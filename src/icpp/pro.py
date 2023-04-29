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
        typer.echo("ERROR: this functionality requires icpp-pro")
        typer.echo(" ")
        typer.echo("For licensing, contact us at icpp@icpp.world")
        typer.echo(" ")
        sys.exit(1)


def is_pro() -> bool:
    """Returns True if running a licensed icpp-pro"""
    return False
