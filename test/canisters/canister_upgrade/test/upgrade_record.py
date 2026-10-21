"""Reads the canister's upgrade record, shared by the before & after tests."""

import re
from pathlib import Path
from typing import Dict, Union

from icpp.smoketest import call_canister_api

# Path to the icp.yaml file
ICP_YAML_PATH = Path(__file__).parent / "../icp.yaml"

# Canister in the icp.yaml file we want to test
CANISTER_NAME = "my_canister"

COUNTS = [
    "init_count",
    "pre_upgrade_count",
    "post_upgrade_count",
    "init_version",
    "pre_upgrade_version",
    "post_upgrade_version",
]
TEXTS = [
    "init_caller",
    "pre_upgrade_caller",
    "post_upgrade_caller",
    "init_self",
    "pre_upgrade_self",
    "post_upgrade_self",
]


def number(record: Dict[str, Union[int, str]], field: str) -> int:
    """A numeric field of the record, typed for arithmetic & comparisons."""
    value = record[field]
    assert isinstance(value, int), f"{field} is not a number: {value!r}"
    return value


def get_upgrade_record(network: str) -> Dict[str, Union[int, str]]:
    """Calls get_upgrade_record and parses the Candid record it returns.

    Fields are matched by name, so the order Candid prints them in (by field
    hash) does not matter.
    """
    response = call_canister_api(
        icp_yaml_path=ICP_YAML_PATH,
        canister_name=CANISTER_NAME,
        canister_method="get_upgrade_record",
        network=network,
    )
    record: Dict[str, Union[int, str]] = {}
    for field in COUNTS:
        match = re.search(rf"\b{field} = (\d+) : nat64", response)
        assert match, f"no {field} in response: {response}"
        record[field] = int(match.group(1))
    for field in TEXTS:
        match = re.search(rf'\b{field} = "([^"]*)"', response)
        assert match, f"no {field} in response: {response}"
        record[field] = match.group(1)
    return record


def call(network: str, method: str, argument: str = "()") -> str:
    """Calls a method of the canister"""
    return call_canister_api(
        icp_yaml_path=ICP_YAML_PATH,
        canister_name=CANISTER_NAME,
        canister_method=method,
        canister_argument=argument,
        network=network,
    )


def write_file(network: str, path: str, mb: int) -> None:
    """Writes a file of mb MiB with write_file"""
    response = call(network, "write_file", f'("{path}", {mb} : nat32)')
    assert response == f"({mb * 1024 * 1024:_} : nat64)", response


def read_file(network: str, path: str) -> Dict[str, str]:
    """The fields of the read_file reply, e.g. {"bytes": "1048576", ...}"""
    text = call(network, "read_file", f'("{path}")').strip('()" \n,')
    return dict(field.split(": ") for field in text.split("; "))


def assert_file(network: str, path: str, mb: int) -> Dict[str, str]:
    """The file holds mb MiB, as written by write_file"""
    fields = read_file(network, path)
    assert fields["bytes"] == str(mb * 1024 * 1024), fields
    assert fields["ok"] == "true", fields
    return fields
