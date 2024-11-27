"""Functions to be used in a pytest test"""

import subprocess
import json
import platform
from pathlib import Path
from typing import Optional, Any, Dict, Tuple
import pytest  # pylint: disable=unused-import

from icpp.run_shell_cmd import run_shell_cmd

DFX = "dfx"
RUN_IN_POWERSHELL = False
if platform.win32_ver()[0]:
    DFX = "wsl --% . ~/.local/share/dfx/env; dfx"
    RUN_IN_POWERSHELL = True


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

    # verify that canister_name is part of dfx.json
    with open(dfx_json_path, "rb") as f:
        data = json.load(f)

    if canister_name not in data["canisters"].keys():
        pytest.fail(
            f"ERROR: canister '{canister_name}' not defined in '{str(dfx_json_path)}'"
        )

    arg = (
        f"{DFX} "
        f" canister "
        f" --network {network} "
        f" {quiet} "
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
            arg,
            capture_output=True,
            cwd=Path(dfx_json_path).parent,
            timeout_seconds=timeout_seconds,
            run_in_powershell=RUN_IN_POWERSHELL,
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
                f"***  {DFX} deploy                       ***\n"
                "*******************************************"
            )
            pytest.exit(msg)
        else:
            response = (
                f"Failed call to api '{canister_method}' of canister '{canister_name}':"
                f"{e.output}"
            )

    return response


def get_canister_url(
    *,
    dfx_json_path: Path,
    canister_name: str,
    network: str = "local",
    url_path: Optional[str] = None,
    timeout_seconds: Optional[int] = None,
) -> str:
    """THIS FUNCTION IS DEPRECATED. DO NOT USE...
    use get_canister_url_with_headers instead

    Returns the url for calling a canister as a Web2.0 HTTP server
    """
    print("get_canister_url has been deprecated since dfx 0.14.1")
    print("instead, use: get_canister_url_with_headers")

    canister_id = get_canister_id(
        dfx_json_path=dfx_json_path,
        canister_name=canister_name,
        network=network,
        timeout_seconds=timeout_seconds,
    )

    if network == "ic":
        url = f"https://{canister_id}"
    else:
        webserver_port = get_local_webserver_port()
        url = f"http://localhost:{webserver_port}"

    # Add the path
    if url_path is not None:
        url = f"{url}/{url_path}"

    # For local network, add the canister_id as a query parameter
    if network == "local":
        url = f"{url}?canisterId={canister_id}"

    return url


def get_canister_url_with_headers(
    *,
    dfx_json_path: Path,
    canister_name: str,
    network: str = "local",
    url_path: Optional[str] = None,
    timeout_seconds: Optional[int] = None,
) -> Tuple[str, Optional[Dict[str, str]]]:
    """Returns the url + headers for calling a canister as a Web2.0 HTTP server"""
    # Updated version for dfx 0.14.1

    canister_id = get_canister_id(
        dfx_json_path=dfx_json_path,
        canister_name=canister_name,
        network=network,
        timeout_seconds=timeout_seconds,
    )

    # See: https://forum.dfinity.org/t/upgrading-to-dfx-0-24-1-breaks-the-http-request-endpoint/36709/13?u=icpp # pylint: disable=line-too-long
    headers = None
    if network == "ic":
        url = f"https://{canister_id}.raw.icp0.io"
    else:
        webserver_port = get_local_webserver_port()
        url = f"http://127.0.0.1:{webserver_port}"
        headers = {"Host": f"{canister_id}.raw.localhost"}

    # Add the path
    if url_path is not None:
        url = f"{url}/{url_path}"

    return url, headers


def get_canister_id(
    *,
    dfx_json_path: Path,
    canister_name: str,
    network: str = "local",
    timeout_seconds: Optional[int] = None,
) -> str:
    """Returns the canister_id of a canister"""

    # verify that canister_name is part of dfx.json
    with open(dfx_json_path, "rb") as f:
        data = json.load(f)

    if canister_name not in data["canisters"].keys():
        pytest.fail(
            f"ERROR: canister '{canister_name}' not defined in '{str(dfx_json_path)}'"
        )

    # Get the canister id
    try:
        arg = f"{DFX} canister --network {network} id {canister_name} "
        response = run_shell_cmd(
            arg,
            capture_output=True,
            cwd=Path(dfx_json_path).parent,
            timeout_seconds=timeout_seconds,
            run_in_powershell=RUN_IN_POWERSHELL,
        )
        canister_id = response.rstrip("\n")
    except subprocess.CalledProcessError as e:
        if "Error: Failed to determine id for canister" in e.output:
            msg = (
                "\n\n"
                f"Failed to get id of canister '{canister_name}'\n\n"
                "*******************************************\n"
                "*** Failed to determine id for canister ***\n"
                "*** Deploy the canister first with:     ***\n"
                f"***  {DFX} deploy                       ***\n"
                "*******************************************"
            )
            pytest.exit(msg)
        else:
            response = f"Failed to get id of canister '{canister_name}':" f"{e.output}"

    return canister_id


def get_local_webserver_port(
    *,
    timeout_seconds: Optional[int] = None,
) -> str:
    """Returns the webserver port of the network"""

    arg = f"{DFX} info webserver-port "
    try:
        response = run_shell_cmd(
            arg,
            capture_output=True,
            timeout_seconds=timeout_seconds,
            run_in_powershell=RUN_IN_POWERSHELL,
        )
        webserver_port = response.rstrip("\n")
    except subprocess.CalledProcessError as e:
        response = f"Failed to get local network's webserver port:" f"{e.output}"

    return webserver_port


def dict_to_candid_text(d: Dict[Any, Any]) -> str:
    """Serializes dict to Candid text to send it over the wire"""
    # pull it through twice, so all " are escaped into \"
    return json.dumps(json.dumps(d))


def network_status(network: str) -> str:
    """Returns the network status."""
    arg = f"{DFX} ping {network}"
    try:
        response = run_shell_cmd(
            arg, capture_output=False, run_in_powershell=RUN_IN_POWERSHELL
        )
    except subprocess.CalledProcessError:
        if network == "local":
            msg = (
                "\n"
                "***************************************\n"
                "*** The local network is not up     ***\n"
                "*** Please start it first with:     ***\n"
                f"***  {DFX} start --clean --background ***\n"
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
    arg = f"{DFX} identity whoami "
    try:
        identity = run_shell_cmd(
            arg,
            capture_output=True,
            timeout_seconds=30,
            run_in_powershell=RUN_IN_POWERSHELL,
        )
        identity = identity.rstrip("\n")
    except subprocess.CalledProcessError as e:
        pytest.fail(f"ERROR: command {arg} failed with error:\n{e.output}")

    return identity


def set_identity(identity: str) -> None:
    """Sets the dfx identity."""
    arg = f"{DFX} identity use {identity}"
    try:
        run_shell_cmd(arg, run_in_powershell=RUN_IN_POWERSHELL)
    except subprocess.CalledProcessError as e:
        pytest.fail(f"ERROR: command {arg} failed with error:\n{e.output}")


def get_principal() -> str:
    """Returns the principal of the current dfx identity."""
    arg = f"{DFX} identity get-principal "
    try:
        principal = run_shell_cmd(
            arg,
            capture_output=True,
            timeout_seconds=30,
            run_in_powershell=RUN_IN_POWERSHELL,
        )
        principal = principal.rstrip("\n")
    except subprocess.CalledProcessError as e:
        pytest.fail(f"ERROR: command {arg} failed with error:\n{e.output}")

    return principal
