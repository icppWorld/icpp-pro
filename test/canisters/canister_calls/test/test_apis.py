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
from typing import Dict, List, Optional, Tuple

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


def test__task_greet(network: str) -> None:
    # A coroutine that awaits another coroutine, which makes the call
    b = principal(network, "canister_b")
    response = call(network, "canister_a", "task_greet", f'(principal "{b}", "C++")')
    assert response == '("Hello, C++!")'


def test__task_nested(network: str) -> None:
    # Two levels of tasks, and a task that finishes without making a call
    b = principal(network, "canister_b")
    response = call(network, "canister_a", "task_nested", f'(principal "{b}", "C++")')
    assert response == '("[Hello, Hello, C++!!]")'


def test__task_then_trap(network: str) -> None:
    # A task traps after its co_await: what happened after the co_await is
    # rolled back, the cleanup frees the frames of the task and of the
    # coroutine awaiting it, and no call stays in flight.
    b = principal(network, "canister_b")
    before = call(network, "canister_a", "get_state")
    response = call(network, "canister_a", "task_then_trap", f'(principal "{b}")')
    assert "trapped on purpose after the co_await" in response
    after = call(network, "canister_a", "get_state")
    b0, a0, d0 = nat64s(before)
    b1, a1, d1 = nat64s(after)
    assert b1 == b0 + 1  # committed by the co_await
    assert a1 == a0  # rolled back with the trap
    assert d1 == d0 + 2  # both frames freed by the cleanup
    assert "0 : nat32" in after


def test__task_not_awaited(network: str) -> None:
    # A task that is never awaited never runs
    assert call(network, "canister_a", "task_not_awaited") == "(0 : nat64)"


def calls_arg(callees: List[str], methods: List[str]) -> str:
    vec_principals = "; ".join(f'principal "{c}"' for c in callees)
    vec_methods = "; ".join(f'"{m}"' for m in methods)
    return f"(vec {{{vec_principals}}}, vec {{{vec_methods}}})"


def test__call_all(network: str) -> None:
    # Several calls at once, to three canisters; the results come back in
    # the order of the calls
    a, b, c = (principal(network, f"canister_{x}") for x in "abc")
    response = call(
        network,
        "canister_a",
        "call_all",
        calls_arg([b, a, c, b], ["greet", "greet", "reject_me", "greet"]),
    )
    assert flatten_candid_text(response) == (
        '(vec { "ok:Hello, C++!"; "ok:Hello, C++!"; "rej:4"; "ok:Hello, C++!" })'
    )


def test__call_all_then_trap(network: str) -> None:
    # The code after co_await IC_CallAll traps: the cleanup frees the frame,
    # and no call stays in flight
    a, b = (principal(network, f"canister_{x}") for x in "ab")
    before = call(network, "canister_a", "get_state")
    response = call(
        network,
        "canister_a",
        "call_all_then_trap",
        calls_arg([b, a], ["greet", "reject_me"]),
    )
    assert "trapped on purpose after the co_await" in response
    after = call(network, "canister_a", "get_state")
    b0, a0, d0 = nat64s(before)
    b1, a1, d1 = nat64s(after)
    assert b1 == b0 + 1 and a1 == a0 and d1 == d0 + 1
    assert "0 : nat32" in after


# ---------------------------------------------------------------------------
# Composite queries


def test__cq_greet(network: str) -> None:
    # A composite query calls a query, on another canister and on itself
    a, b = (principal(network, f"canister_{x}") for x in "ab")
    for callee in (b, a):
        response = call(
            network, "canister_a", "cq_greet", f'(principal "{callee}", "C++")'
        )
        assert response == '("Hello, C++!")'


def test__cq_all(network: str) -> None:
    # Several calls at once, from a composite query
    a, b, c = (principal(network, f"canister_{x}") for x in "abc")
    vec = "; ".join(f'principal "{x}"' for x in (a, b, c))
    response = call(network, "canister_a", "cq_all", f"(vec {{{vec}}})")
    assert flatten_candid_text(response) == (
        '(vec { "ok:Hello, C++!"; "ok:Hello, C++!"; "ok:Hello, C++!" })'
    )


def test__cq_call_update(network: str) -> None:
    # A composite query can only call query and composite query methods
    b = principal(network, "canister_b")
    response = call(network, "canister_a", "cq_call_update", f'(principal "{b}")')
    assert response.startswith('("rej:'), response


def test__call_cq(network: str) -> None:
    # An update can't call a composite query: that would be replicated mode
    b = principal(network, "canister_b")
    response = call(network, "canister_a", "call_cq", f'(principal "{b}")')
    assert response.startswith('("rej:'), response


def test__cq_in_callback_probe(network: str) -> None:
    # Only the top-level query sees the data certificate; nothing is replicated
    b = principal(network, "canister_b")
    response = call(
        network, "canister_a", "cq_in_callback", f'(principal "{b}", "probe")'
    )
    assert response == '("certificate: 1 then 0; replicated: 0 then 0")'


def test__cq_in_callback_refused(network: str) -> None:
    # Timers, certified data and cycles are not available in a composite query
    b = principal(network, "canister_b")
    for what, expected in (
        ("timer", "CanisterCompositeReplyCallback"),
        ("certify", "CanisterCompositeReplyCallback"),
        ("cycles", "a composite query can't send cycles"),
    ):
        response = call(
            network, "canister_a", "cq_in_callback", f'(principal "{b}", "{what}")'
        )
        assert expected in response, f"{what}: {response}"


def test__cq_via(network: str) -> None:
    # A composite query calls a composite query of another canister:
    # python -> a.cq_via -> b.info_cq -> c.greet_q
    b, c = (principal(network, f"canister_{x}") for x in "bc")
    response = call(
        network, "canister_a", "cq_via", f'(principal "{b}", principal "{c}")'
    )
    assert response == '("ok:Hello, info!")'


def timer_call(network: str, method: str) -> str:
    # A timer on canister_c calls canister_a.<method>(canister_b), and
    # timer_result reports the outcome once the timer fired
    a, b = (principal(network, f"canister_{x}") for x in "ab")
    argument = f'(principal "{a}", "{method}", principal "{b}")'
    assert call(network, "canister_c", "timer_call", argument) == "()"
    for _ in range(30):
        response = call(network, "canister_c", "timer_result")
        if response != '("pending")':
            return response
        time.sleep(1)
    return response


def test__timer_calls_cq(network: str) -> None:
    # A timer runs in replicated mode: its call to a composite query is
    # rejected with code 5 (CanisterError)
    assert timer_call(network, "info_cq") == '("rej:5")'


def test__timer_calls_update_variant(network: str) -> None:
    # The same logic, exported as an update, can be called from a timer
    assert timer_call(network, "info") == '("ok:Hello, info!")'
    # and users still call it as a composite query
    b = principal(network, "canister_b")
    response = call(network, "canister_a", "info_cq", f'(principal "{b}")')
    assert response == '("ok:Hello, info!")'


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


# ---------------------------------------------------------------------------
# A long computation that only completes with yield_fresh_budget()

INSTRUCTION_LIMIT = 40_000_000_000  # of one update message
LCG_A = 6364136223846793005
LCG_C = 1442695040888963407


def lcg(x: int, n: int) -> int:
    """x = a * x + c (mod 2^64), n times, by composing the step with itself."""
    mask = (1 << 64) - 1
    a, c = LCG_A, LCG_C
    total_a, total_c = 1, 0
    while n:
        if n & 1:
            total_a, total_c = (a * total_a) & mask, (a * total_c + c) & mask
        a, c = (a * a) & mask, (a * c + c) & mask
        n >>= 1
    return (total_a * x + total_c) & mask


def compute(  # pylint: disable=too-many-arguments, too-many-positional-arguments
    network: str, job: int, seed: int, n: int, chunk: int, reload: bool = True
) -> List[int]:
    """(result, chunks, call_instructions, max_message_instructions)"""
    response = call(
        network,
        "canister_a",
        "compute",
        f"({job} : nat32, {seed} : nat64, {n} : nat64, {chunk} : nat64, "
        f"{str(reload).lower()})",
        timeout_seconds=900,
    )
    return [
        int(token.split(":")[0].strip().replace("_", ""))
        for token in response.strip("()\n ").split(",")
        if ":" in token
    ]


def steps_for(network: str, instructions: int) -> int:
    """The number of LCG steps that executes at least `instructions`
    instructions: calibrated with a short run, plus a 2% margin."""
    calibration = 100_000_000
    used = compute(network, 0, 1, calibration, INSTRUCTION_LIMIT // 2)[2]
    return instructions * calibration * 102 // (used * 100) + 1


def test__compute_150b_instructions(network: str) -> None:
    # 150B instructions: almost 4x the limit of one message. It completes,
    # in chunks of at most 20B, each followed by a yield.
    n = steps_for(network, 150_000_000_000)
    result, chunks, call_instructions, max_message = compute(
        network, 1, 7, n, 20_000_000_000
    )
    assert result == lcg(7, n)
    assert call_instructions >= 150_000_000_000
    assert max_message < INSTRUCTION_LIMIT
    assert chunks >= 150_000_000_000 // 20_000_000_000


def test__compute_no_yield_traps(network: str) -> None:
    # The same computation without yields, at 50B instructions: it exceeds
    # the instruction limit of one message
    n = steps_for(network, 50_000_000_000)
    response = call(
        network,
        "canister_a",
        "compute_no_yield",
        f"(7 : nat64, {n} : nat64)",
        timeout_seconds=900,
    )
    assert "instructions" in response and "limit" in response, response


def run_jobs(network: str, reload: bool) -> Tuple[int, List[int]]:
    """Starts 8 jobs at the same time, each ~5B instructions in chunks of
    0.5B. Returns how many got the right result, and the run log."""
    jobs = range(1, 9)
    n = steps_for(network, 5_000_000_000)
    assert call(network, "canister_a", "clear_run_log") == "()"
    results: Dict[int, int] = {}

    def run(job: int) -> None:
        results[job] = compute(network, job, 1000 + job, n, 500_000_000, reload)[0]

    threads = [threading.Thread(target=run, args=(job,)) for job in jobs]
    for thread in threads:
        thread.start()
    for thread in threads:
        thread.join(timeout=900)
    # A job that failed or did not finish must fail the test, not count as a
    # wrong result: that would pass the test without reload for the wrong
    # reason
    assert not any(thread.is_alive() for thread in threads), "jobs did not finish"
    assert set(results) == set(jobs), f"jobs failed: {set(jobs) - set(results)}"
    right = sum(1 for job in jobs if results.get(job) == lcg(1000 + job, n))
    log_text = call(network, "canister_a", "get_run_log")
    log = [int(x.split(":")[0]) for x in log_text.split("{")[1].split(";") if ":" in x]
    return right, log


def test__compute_many_users(network: str) -> None:
    # 8 jobs at the same time: their chunks interleave, and every job gets
    # the right result, because every chunk reloads its own state first
    right, log = run_jobs(network, reload=True)
    assert right == 8
    switches = sum(1 for i in range(1, len(log)) if log[i] != log[i - 1])
    assert switches > 8, f"the chunks did not interleave: {log}"


def test__compute_many_users_without_reload(network: str) -> None:
    # The proof that the chunks interleave: when a chunk does not reload its
    # state, another job's chunk changed it in between, and results are wrong
    right, _ = run_jobs(network, reload=False)
    assert right < 8


# ---------------------------------------------------------------------------


def icp_canister(
    network: str, identity: str, command: str, timeout_seconds: int = 300
) -> subprocess.CompletedProcess[str]:
    """Runs `icp canister <command>` in this project."""
    project = ICP_YAML_PATH.parent.resolve()
    return subprocess.run(
        f"{ICP} canister {command} "
        f"--environment {network} --identity {identity} --project-root-override {project} "
        "< /dev/null",
        shell=True,
        capture_output=True,
        text=True,
        check=False,
        timeout=timeout_seconds,
    )


def upgrade(
    network: str, identity: str, canister: str
) -> subprocess.CompletedProcess[str]:
    return icp_canister(
        network,
        identity,
        f"install {canister} --mode upgrade --wasm {WASM_PATH.resolve()}",
    )


def canister_status(network: str, identity: str, canister: str) -> str:
    """Running, Stopping or Stopped"""
    result = icp_canister(network, identity, f"status {canister}")
    assert result.returncode == 0, result.stdout + result.stderr
    for status in ("Running", "Stopping", "Stopped"):
        if status in result.stdout:
            return status
    raise AssertionError(result.stdout)


def test__stop_during_yield_loop(network: str, identity: str) -> None:
    # A stop while a yield loop runs: yield_fresh_budget returns
    # CanisterStopping, the loop replies with what it has, and the stop
    # completes instead of waiting for the loop to end. The reply proves it;
    # how long the stop takes depends on the runner (66s seen on CI), so the
    # only time limit is the stop's timeout, which a hang would hit.
    canister = "canister_c"
    responses: List[str] = []

    def loop() -> None:
        # Record a failed call too, so the test reports it
        try:
            responses.append(
                call(
                    network,
                    canister,
                    "yield_until_stop",
                    "(1_000_000 : nat32)",
                    timeout_seconds=300,
                )
            )
        except Exception as e:  # pylint: disable=broad-exception-caught
            responses.append(f"the call failed: {e!r}")

    # A daemon thread: a call that never returns can't keep pytest alive
    looping = threading.Thread(target=loop, daemon=True)
    seconds = 0.0
    try:
        looping.start()
        time.sleep(5)
        assert looping.is_alive(), "the yield loop ended before the stop"

        start = time.time()
        stopped = icp_canister(
            network, identity, f"stop {canister}", timeout_seconds=120
        )
        seconds = time.time() - start
        looping.join(timeout=60)
        assert stopped.returncode == 0, stopped.stdout + stopped.stderr
        assert not looping.is_alive() and responses, "the call did not return"
        assert "stopping after" in responses[0], responses[0]
        assert int(responses[0].split("after ")[1].split(" ")[0]) > 0
        assert canister_status(network, identity, canister) == "Stopped"
    finally:
        # Also after a failed or timed-out stop: the other tests need it
        # running. An assert here would hide the original failure.
        started = icp_canister(network, identity, f"start {canister}")
        if started.returncode != 0:
            print(f"Could not start {canister}: {started.stdout}{started.stderr}")
    assert started.returncode == 0, started.stdout + started.stderr
    assert canister_status(network, identity, canister) == "Running"
    print(f"The stop took {seconds:.1f}s: {responses[0]}")


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
