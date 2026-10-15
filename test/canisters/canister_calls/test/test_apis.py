"""Test inter-canister calls (ic_async.h)

The same wasm is deployed as canister_a, canister_b and canister_c. First
deploy all three, then run:

$ pytest --network=local
"""

# pylint: disable=missing-function-docstring, unused-import, wildcard-import, unused-wildcard-import

import subprocess
import threading
import time
from pathlib import Path
from typing import List, Optional

from icpp.run_shell_cmd import run_shell_cmd
from icpp.smoketest import ICP, call_canister_api, flatten_candid_text, get_canister_id

# Path to the icp.yaml file
ICP_YAML_PATH = Path(__file__).parent / "../icp.yaml"
WASM_PATH = Path(__file__).parent / "../build/my_canister.wasm"


def call(
    network: str,
    canister: str,
    method: str,
    argument: str = "()",
    timeout_seconds: int = 30,
    query: Optional[bool] = None,
) -> str:
    return call_canister_api(
        icp_yaml_path=ICP_YAML_PATH,
        canister_name=canister,
        canister_method=method,
        canister_argument=argument,
        network=network,
        timeout_seconds=timeout_seconds,
        query=query,
    )


def principal(network: str, canister: str) -> str:
    return get_canister_id(
        canister_name=canister, icp_yaml_path=ICP_YAML_PATH, network=network
    )


def nat64s(response: str) -> List[int]:
    """The nat64 values of a Candid text response, in order."""
    return [
        int(token.split(":")[0].replace("_", ""))
        for token in response.replace("(", " ").replace(")", " ").split(",")
        if ": nat64" in token
    ]


def test__call_greet_b(network: str) -> None:
    b = principal(network, "canister_b")
    response = call(network, "canister_a", "call_greet", f'(principal "{b}", "C++")')
    assert response == '("Hello, C++!")'


def test__call_greet_self(network: str) -> None:
    a = principal(network, "canister_a")
    response = call(network, "canister_a", "call_greet", f'(principal "{a}", "me")')
    assert response == '("Hello, me!")'


def test__call_reject(network: str) -> None:
    b = principal(network, "canister_b")
    response = call(network, "canister_a", "call_reject", f'(principal "{b}")')
    assert response == '("4:rejected on purpose")'


def test__call_trap(network: str) -> None:
    b = principal(network, "canister_b")
    response = call(network, "canister_a", "call_trap", f'(principal "{b}")')
    assert response == '("5")'


def test__call_then_trap(network: str) -> None:
    # The code after the co_await traps: its state changes are rolled back,
    # the cleanup frees the frame, and no call stays in flight.
    b = principal(network, "canister_b")
    before = call(network, "canister_a", "get_state")
    response = call(network, "canister_a", "call_then_trap", f'(principal "{b}")')
    assert "trapped on purpose after the co_await" in response
    after = call(network, "canister_a", "get_state")
    b0, a0, d0 = nat64s(before)
    b1, a1, d1 = nat64s(after)
    assert b1 == b0 + 1  # committed by the co_await
    assert a1 == a0  # rolled back with the trap
    assert d1 == d0 + 1  # the frame was freed by the cleanup
    assert "0 : nat32" in after


def test__call_with_cycles(network: str) -> None:
    b = principal(network, "canister_b")
    response = call(
        network,
        "canister_a",
        "call_with_cycles",
        f'(principal "{b}", 1_000_000 : nat64)',
    )
    assert response == "(500_000 : nat64, 500_000 : nat64)"


def test__call_sequence(network: str) -> None:
    # Two calls, one after the other: canister_b.greet, then canister_a.greet
    b = principal(network, "canister_b")
    response = call(network, "canister_a", "call_sequence", f'(principal "{b}", "C++")')
    assert response == '("Hello, Hello, C++!!")'


def test__call_chain_a_b_c(network: str) -> None:
    # A chain: canister_b calls canister_c before it replies to canister_a
    a, b, c = (principal(network, f"canister_{x}") for x in "abc")
    response = call(
        network,
        "canister_a",
        "call_chain",
        f'(vec {{principal "{b}"; principal "{c}"}}, "C++")',
    )
    assert f'"{a} > {b} > {c}: Hello, C++!"' in flatten_candid_text(response)


def test__call_chain_a_b_a(network: str) -> None:
    # A call back into the caller: canister_b calls canister_a, while
    # canister_a is waiting for canister_b
    a, b = (principal(network, f"canister_{x}") for x in "ab")
    response = call(
        network,
        "canister_a",
        "call_chain",
        f'(vec {{principal "{b}"; principal "{a}"}}, "C++")',
    )
    assert f'"{a} > {b} > {a}: Hello, C++!"' in flatten_candid_text(response)


def test__call_oneway(network: str) -> None:
    b = principal(network, "canister_b")
    response = call(network, "canister_a", "call_oneway", f'(principal "{b}")')
    assert response == "(true)"


def test__call_bounded_replied(network: str) -> None:
    b = principal(network, "canister_b")
    response = call(
        network,
        "canister_a",
        "call_bounded",
        f'(principal "{b}", 60 : nat32, 2 : nat32)',
    )
    assert response == '("replied")'


def test__yield_loop(
    network: str, principal: str
) -> None:  # pylint: disable=redefined-outer-name
    # The caller survives the yields
    response = flatten_candid_text(
        call(network, "canister_a", "yield_loop", "(5 : nat32)")
    )
    assert f'"{principal} after 5 yields"' in response


def test__budget_probe(network: str) -> None:
    # Each yield resumes in a new message: the per-message instruction counter
    # starts again, while the call-context counter keeps growing.
    response = call(network, "canister_a", "budget_probe", "(4 : nat32)")
    vectors = [v.split("}")[0] for v in response.split("vec {")[1:]]
    message = [
        int(x.split(":")[0].strip().replace("_", ""))
        for x in vectors[0].split(";")
        if ":" in x
    ]
    total = [
        int(x.split(":")[0].strip().replace("_", ""))
        for x in vectors[1].split(";")
        if ":" in x
    ]
    assert len(message) == 4 and len(total) == 4
    assert all(total[i + 1] > total[i] for i in range(3))
    assert total[-1] > sum(message[1:]) // 2
    assert max(message[1:]) < total[-1]


def upgrade(
    network: str, identity: str, canister: str
) -> subprocess.CompletedProcess[str]:
    project = ICP_YAML_PATH.parent.resolve()
    return subprocess.run(
        f"{ICP} canister install {canister} --mode upgrade --wasm {WASM_PATH.resolve()} "
        f"--environment {network} --identity {identity} --project-root-override {project} "
        "< /dev/null",
        shell=True,
        capture_output=True,
        text=True,
        check=False,
    )


def test__upgrade_waits_while_call_in_flight(network: str, identity: str) -> None:
    # icp-cli stops a canister before upgrading it, and a stopping canister
    # only becomes stopped once its calls in flight returned: the upgrade
    # waits for the held call. (A deploy tool that does not stop the canister
    # first meets canister_pre_upgrade, which refuses while calls are in
    # flight - covered by the native tests.)
    # While canister_b holds the call open, it self-yields every round, which
    # slows the local replica down: hence the long timeouts, and update calls
    # (query=False) for get_state.
    b = principal(network, "canister_b")
    assert call(network, "canister_a", "start_held_call", f'(principal "{b}")') == "()"
    state = call(network, "canister_a", "get_state", timeout_seconds=120, query=False)
    assert "1 : nat32" in state

    results: List[subprocess.CompletedProcess[str]] = []
    upgrading = threading.Thread(
        target=lambda: results.append(upgrade(network, identity, "canister_a"))
    )
    upgrading.start()
    time.sleep(5)
    assert upgrading.is_alive(), "the upgrade did not wait for the call in flight"

    assert call(network, "canister_b", "release", timeout_seconds=120) == "()"
    upgrading.join(timeout=180)
    assert not upgrading.is_alive()
    assert results[0].returncode == 0, results[0].stdout + results[0].stderr
    state = call(network, "canister_a", "get_state", timeout_seconds=120, query=False)
    assert "0 : nat32" in state
