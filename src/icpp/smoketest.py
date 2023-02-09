"""Functions to be used in a pytest test"""

import subprocess
import json
from pathlib import Path
from typing import Optional, Any
import pytest  # pylint: disable=unused-import

from icpp.run_shell_cmd import run_shell_cmd


def call_canister_api(
    *,
    dfx_json_path: Path,
    canister_name: str,
    canister_method: str,
    canister_argument: Optional[str] = None,
    canister_input: str = "idl",
    canister_output: str = "idl",
    network: str = "local",
) -> str:
    """Calls a canister method"""

    # verify that canister_name is part of dfx.json
    with open(dfx_json_path, "rb") as f:
        data = json.load(f)

    if canister_name not in data["canisters"].keys():
        pytest.fail(
            f"ERROR: canister '{canister_name}' not defined in '{str(dfx_json_path)}'"
        )

    arg = (
        f"dfx "
        f" canister "
        f" --network {network} "
        f" call "
        f" --type {canister_input} "
        f" --output {canister_output} "
        f" {canister_name} "
        f" {canister_method} "
    )

    if canister_argument is None:
        arg += " '()' "
    else:
        arg += f" '{canister_argument}' "
    try:
        # response = run_shell_cmd(arg, cwd=Path(dfx_json_path).parent)
        response = run_shell_cmd(
            arg, capture_output=True, cwd=Path(dfx_json_path).parent
        )
        response = response.rstrip("\n")
    except subprocess.CalledProcessError as e:
        if "Error: Failed to determine id for canister" in e.output:
            msg = (
                "\n\n"
                f"FAIL: Call to api '{canister_method}' of canister "
                f"'{canister_name}'\n\n"
                "*******************************************\n"
                "*** Failed to determine id for canister ***\n"
                "*** Deploy the canister first with:     ***\n"
                "***  dfx deploy                         ***\n"
                "*******************************************"
            )
            pytest.exit(msg)
        else:
            response = (
                f"Failed call to api '{canister_method}' of canister '{canister_name}':"
                f"{e.output}"
            )

    return response


def dict_to_candid_text(d: dict[Any, Any]) -> str:
    """Serializes dict to Candid text to send it over the wire"""
    # pull it through twice, so all " are escaped into \"
    return json.dumps(json.dumps(d))


def network_status(network: str) -> str:
    """Returns the network status."""
    arg = f"dfx ping {network}"
    try:
        response = run_shell_cmd(arg, capture_output=False)
    except subprocess.CalledProcessError:
        if network == "local":
            msg = (
                "\n"
                "***************************************\n"
                "*** The local network is not up     ***\n"
                "*** Please start it first with:     ***\n"
                "***  dfx start --clean --background ***\n"
                "***************************************"
            )
        else:
            msg = (
                "\n"
                "*******************************\n"
                "*** The IC is not up        ***\n"
                "*** Please try again later  ***\n"
                "*******************************"
            )
        pytest.exit(msg)

    return response


def get_identity() -> str:
    """Returns the current dfx identity."""
    arg = "dfx identity whoami "
    try:
        identity = run_shell_cmd(arg)
    except subprocess.CalledProcessError as e:
        pytest.fail(f"ERROR: command {arg} failed with error:\n{e.output}")

    return identity


def set_identity(identity: str) -> None:
    """Sets the dfx identity."""
    arg = f"dfx identity use {identity}"
    try:
        run_shell_cmd(arg)
    except subprocess.CalledProcessError as e:
        pytest.fail(f"ERROR: command {arg} failed with error:\n{e.output}")


def get_principal() -> str:
    """Returns the principal of the current dfx identity."""
    arg = "dfx identity get principal "
    try:
        principal = run_shell_cmd(arg)
    except subprocess.CalledProcessError as e:
        pytest.fail(f"ERROR: command {arg} failed with error:\n{e.output}")

    return principal
