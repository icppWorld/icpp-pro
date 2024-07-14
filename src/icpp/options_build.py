"""Handles validation of --compile option """

from typing import Optional
import typer

option_to_compile_values = ["all", "mine", "mine-no-lib"]
option_to_compile_values_string = f"[{'/'.join(option_to_compile_values)}]"

option_generate_bindings_values = ["yes", "no"]
option_generate_bindings_values_string = (
    f"[{'/'.join(option_generate_bindings_values)}]"
)


def config_callback(ctx: typer.Context, value: str) -> Optional[str]:
    """--config [...]"""
    # Handle auto complete
    if ctx.resilient_parsing:
        return None

    return value


def to_compile_callback(ctx: typer.Context, value: str) -> Optional[str]:
    """--to-compile [...]"""
    # Handle auto complete
    if ctx.resilient_parsing:
        return None

    if value not in option_to_compile_values:
        msg = f"'{value}'\nValid values are: {option_to_compile_values_string}"
        raise typer.BadParameter(msg)

    return value


def generate_bindings_callback(ctx: typer.Context, value: str) -> Optional[str]:
    """--generate-bindings [...]"""
    # Handle auto complete
    if ctx.resilient_parsing:
        return None

    if value not in option_generate_bindings_values:
        msg = f"'{value}'\nValid values are: {option_generate_bindings_values_string}"
        raise typer.BadParameter(msg)

    return value
