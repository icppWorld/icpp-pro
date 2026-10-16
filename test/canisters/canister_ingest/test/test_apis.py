"""Ingest a state with update self-calls, then ask about it with a query

First deploy the canister, then run:

$ pytest --network=local
"""

# pylint: disable=missing-function-docstring, unused-import, wildcard-import, unused-wildcard-import

from pathlib import Path

from icpp.smoketest import call_canister_api

# Path to the icp.yaml file
ICP_YAML_PATH = Path(__file__).parent / "../icp.yaml"

MESSAGES = '(vec {"apple pie"; "apple tart"; "cherry"})'


def call(network: str, method: str, argument: str = "()") -> str:
    return call_canister_api(
        icp_yaml_path=ICP_YAML_PATH,
        canister_name="canister_ingest",
        canister_method=method,
        canister_argument=argument,
        network=network,
        timeout_seconds=120,
    )


def test__ingest_then_ask(network: str) -> None:
    assert call(network, "reset") == "()"
    # Three update self-calls, each appending a message to the state file
    assert call(network, "ingest", MESSAGES) == "(28 : nat64)"

    # A query reads the file that the self-calls wrote
    response = call(network, "ask", '("apple")')
    assert response.startswith(
        '("3 messages, 2 mention apple; instructions: '
    ), response

    # So does a query that a composite query calls on the canister itself
    response = call(network, "ask_cq", '("apple")')
    assert response.startswith('("via ask: 3 messages, 2 mention apple'), response


def test__ingest_appends(network: str) -> None:
    assert call(network, "reset") == "()"
    assert call(network, "ingest", MESSAGES) == "(28 : nat64)"
    assert call(network, "ingest", '(vec {"apple crumble"})') == "(42 : nat64)"
    response = call(network, "ask", '("apple")')
    assert response.startswith('("4 messages, 3 mention apple'), response


def test__store_only_by_self(network: str) -> None:
    response = call(network, "store", '("not from the canister")')
    assert "only the canister itself may call store" in response, response


def test__query_budget(network: str) -> None:
    # One query has the instruction limit of a single message: 5 billion
    response = call(network, "burn_q", "(4_900_000_000 : nat64)")
    assert response.startswith("(4_900_"), response
    response = call(network, "burn_q", "(6_000_000_000 : nat64)")
    assert "exceeded the limit of 5000000000 instructions" in response, response


def test__composite_query_budget_is_shared(network: str) -> None:
    # A composite query and the queries it calls share one budget of 5 billion
    # instructions: a self-call does not give a fresh one
    response = call(network, "cq_burn", "(2 : nat32, 2_000_000_000 : nat64)")
    assert response.startswith('("parts: 2 of 2; burn_q: 2000'), response
    response = call(network, "cq_burn", "(3 : nat32, 2_000_000_000 : nat64)")
    assert "Composite query calls exceeded the instruction limit" in response, response


def test__ingest_refuses_newline(network: str) -> None:
    # A message with a newline would read back as two: the whole batch is
    # refused before anything is stored
    assert call(network, "reset") == "()"
    response = call(network, "ingest", '(vec {"apple pie"; "apple\\ncherry"})')
    assert "a message can't contain a newline" in response, response
    response = call(network, "ask", '("apple")')
    assert response.startswith('("0 messages, 0 mention apple'), response
