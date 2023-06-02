"""Functions to be used in a pytest test"""

# pylint: disable=unused-import, unused-argument

import subprocess
import json
from pathlib import Path
from typing import Optional, Any
import pytest  # pylint: disable=unused-import

from icpp.run_shell_cmd import run_shell_cmd
from icpp import pro


def call_canister_api(
    *,
    dfx_json_path: Path,
    canister_name: str,
    canister_method: str,
    canister_argument: Optional[str] = None,
    canister_input: str = "idl",
    canister_output: str = "idl",
    network: str = "local",
    quiet: str = "-qq",  # limits dfx to errors only
    timeout_seconds: Optional[int] = None,
) -> str:
    """Calls a canister method"""
    pro.exit_if_not_pro("smoketesting with pytest")
    return ""


def dict_to_candid_text(d: dict[Any, Any]) -> str:
    """Serializes dict to Candid text to send it over the wire"""
    pro.exit_if_not_pro("smoketesting with pytest")
    return ""


def network_status(network: str) -> str:
    """Returns the network status."""
    pro.exit_if_not_pro("smoketesting with pytest")
    return ""


def get_identity() -> str:
    """Returns the current dfx identity."""
    pro.exit_if_not_pro("smoketesting with pytest")
    return ""


def set_identity(identity: str) -> None:
    """Sets the dfx identity."""
    pro.exit_if_not_pro("smoketesting with pytest")


def get_principal() -> str:
    """Returns the principal of the current dfx identity."""
    pro.exit_if_not_pro("smoketesting with pytest")
    return ""
