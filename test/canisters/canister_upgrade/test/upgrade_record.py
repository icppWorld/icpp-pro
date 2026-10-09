"""Reads the canister's upgrade record, shared by the before & after tests."""

import re
from pathlib import Path
from typing import Dict, Union

from icpp.smoketest import call_canister_api

# Path to the icp.yaml file
ICP_YAML_PATH = Path(__file__).parent / "../icp.yaml"

# Canister in the icp.yaml file we want to test
CANISTER_NAME = "my_canister"

COUNTS = ["init_count", "pre_upgrade_count", "post_upgrade_count"]
TEXTS = [
    "init_caller",
    "pre_upgrade_caller",
    "post_upgrade_caller",
    "init_self",
    "pre_upgrade_self",
    "post_upgrade_self",
]


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
