"""Handles validation of --compile option"""

from typing import Annotated, Optional
import typer

# Where a built icpp-candid library is saved & reused (see candid_library.py)
CandidLibraryDirOption = Annotated[
    Optional[str],
    typer.Option(
        "--candid-library-dir",
        envvar="ICPP_CANDID_LIBRARY_DIR",
        help=(
            "Reuse the icpp-candid library saved in this folder for the same "
            "compile commands and icpp-pro version, or build it and save it "
            "there. The folder's owner guarantees the sources do not change "
            "while it is used (e.g. a fresh folder per CI run)."
        ),
    ),
]

option_to_compile_values = ["all", "mine", "mine-no-lib"]
# pylint: disable=invalid-name
option_to_compile_values_string = f"[{'/'.join(option_to_compile_values)}]"

option_generate_bindings_values = ["yes", "no"]
# pylint: disable=invalid-name
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
