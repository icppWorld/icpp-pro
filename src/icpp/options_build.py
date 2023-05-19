"""Handles validation of --compile option """
from typing import Optional
import typer
from icpp import pro

option_to_compile_values = ["all", "mine"]
option_to_compile_values_string = f"[{'/'.join(option_to_compile_values)}]"


def to_compile_callback(ctx: typer.Context, value: str) -> Optional[str]:
    """--to-compile [...]"""
    # Handle auto complete
    if ctx.resilient_parsing:
        return None

    if value not in option_to_compile_values:
        msg = f"'{value}'\nValid values are: {option_to_compile_values_string}"
        raise typer.BadParameter(msg)

    if value != "all":
        pro.exit_if_not_pro(f"--to-compile {value}")

    return value
