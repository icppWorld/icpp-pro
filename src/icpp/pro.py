"""
Handles licensing of icpp-pro

********************************************************************************
* IT IS NOT ALLOWED TO MODIFY THIS FILE OR ANY FILE THAT CALLS THESE FUNCTIONS *
********************************************************************************

"""
import sys
from typing import Optional
import typer


def exit_if_not_pro(capability: Optional[str] = None) -> None:
    """Exit if not running a licensed icpp-pro."""
    if capability is None:
        msg = "this command"
    else:
        msg = f"'{capability}'"
    if not is_pro():
        typer.echo("--")
        typer.echo(f"ERROR: {msg} requires icpp-pro")
        typer.echo(" ")
        typer.echo("You can just upgrade with: `pip install icpp-pro`")
        typer.echo("But please be a sport, and donate a little. Thank YOU!")
        typer.echo(
            "For details, see: https://docs.icpp.world/index.html#donations-welcome"
        )
        typer.echo(" ")
        sys.exit(1)


def is_pro() -> bool:
    """Returns True if running a licensed icpp-pro"""
    return False
