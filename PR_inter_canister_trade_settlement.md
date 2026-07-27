# Inter-canister trade settlement — `IC_Call` helper

## Summary

Adds a high-level C++ API for inter-canister calls to `icpp-pro`. Before this
PR, an `icpp-pro` canister could only be **called**; it had no clean way to
**call out** to another canister without writing raw `ic0_call_*` imports and
manually shepherding env cookies through reply/reject WASM callbacks.

This is the prerequisite for the **inter-canister trade settlement** work in
the ICSoccerWorld marketplace (`ICMarketplace`), where every `buy_listing`,
`accept_offer`, `place_bid` and `settle_auction` needs to:

1. Pull payment via `icrc2_transfer_from` on a currency ledger.
2. Move the NFT via `icrc37_transfer_from` on the collection canister.
3. Disburse `seller_receives + royalty + protocol_fee` via three
   `icrc1_transfer` calls.
4. Compensate (refund) on any partial failure.

All of those require **async outbound calls from an update handler**, which
this PR makes ergonomic.

## What the PR adds

| File | Purpose |
|---|---|
| `src/icpp/ic/icapi/ic_call.h` | Public API: `IC_Call::call(...)`, `IC_Call::raw_call(...)`, `IC_CallBuilder`, `CallReject`, `CallRejectCode`. |
| `src/icpp/ic/icapi/ic_call.cpp` | Implementation: env-cookie registry, exported `__icpp_call_reply_trampoline` / `__icpp_call_reject_trampoline` WASM callbacks, Candid arg serialisation, principal-bytes extraction. |
| `test/canisters/canister_call/` | End-to-end test canister that performs a **self-call**: `ping_self("hi")` issues an inter-canister call to its own `echo(text)` method and finishes the original message from inside the `on_reply` callback. |

No build-script changes are needed — `config_default.py` already globs
`ic/icapi/*.cpp` and `ic/icapi/*.h` automatically.

## API surface

```cpp
// Low-level
uint32_t IC_Call::call(
    const CandidTypePrincipal &callee,
    const std::string         &method,
    const CandidArgs          &args,
    std::function<void(const VecBytes&)>     on_reply,
    std::function<void(const CallReject&)>   on_reject);

// Textual-principal convenience overload
uint32_t IC_Call::call(const std::string &callee_text, ...);

// Fluent builder
IC_CallBuilder(callee, "icrc1_transfer")
    .with_args(args)
    .on_reply ([](const VecBytes& b) { /* decode + continue */ })
    .on_reject([](const CallReject& r){ /* refund + abort   */ })
    .perform();
```

Return value of `perform()` / `call()` is the `ic0.call_perform` system code:
**0 = queued**, non-zero = system-level failure (call was never sent, no
callback will run).

## Design notes

* **Env-cookie registry** — every outbound call gets a monotonically
  increasing `uint32_t` cookie. The cookie is passed unmodified through
  `ic0.call_new` as the reply/reject env and is used to find the right C++
  handler in an `std::unordered_map<uint32_t, Pending>` when the IC fires
  the trampoline.

* **Trampolines** — `__icpp_call_reply_trampoline(env)` and
  `__icpp_call_reject_trampoline(env)` are exported as `canister_callback`
  symbols. They're the only WASM-table entries the IC ever sees; they
  immediately dispatch into the C++ map.

* **No state on the message stack** — the original update message returns
  before the callee replies. State that needs to survive must live on the
  canister heap (typically the `*Storage` singleton, mirroring how the
  marketplace already keeps its in-flight trades).

* **Failure model** — if `ic0.call_perform` returns non-zero, the cookie is
  removed before `raw_call` returns. If the callee traps or rejects, the
  registered `on_reject` is invoked with the IC's reject code + message.

* **Upgrade safety** — `IC_Call::clear_pending()` drops every callback. User
  code should call it from their `pre_upgrade` hook (and persist any
  trade-state they want to resume in `post_upgrade`). This matches the
  existing pattern for `set_timer` continuations.

## Test plan

The new `canister_call` test canister exercises the round trip end-to-end:

1. `pytest --network=local` deploys the canister and calls `echo("hello")`
   directly to confirm the callee works.
2. Calls `ping_self("ic_call_works")` and polls `get_last_echoed` until the
   echo round-trips through the inter-canister reply callback.
3. Confirms `get_pending_count` is `0` after the call settles, proving the
   env cookie was freed (no leak).

Run from the canister directory:

```bash
cd test/canisters/canister_call
icpp build-wasm
dfx start --clean --background
dfx deploy
pytest --network=local
```

## Backwards compatibility

* **Additive only.** No existing icpp-pro APIs are touched.
* No new public exports clash with user canisters — both trampolines are
  prefixed `__icpp_`.
* WASM size impact: a few hundred bytes for the map + dispatcher; trampolines
  are inlinable.

## Downstream work that depends on this PR

The `ICMarketplace` canister (https://github.com/ktimam/ICSoccerWorldServer)
has a parallel branch `feature/phase-1b-inter-canister-trade-settlement`
that:

1. Adds an `InFlightTrade` state machine to `MarketplaceStorage`.
2. Replaces the `TODO Phase 1B` comment blocks in `MarketplaceServer.cpp`
   (in `buy_listing`, `make_offer`, `accept_offer`, `cancel_offer`,
   `place_bid`, `settle_auction`) with real `IC_Call` invocations.
3. Adds idempotent `memo`-tagged retries (`memo = sha256(trade_id || step)`).
4. Adds an end-to-end test that asserts ledger balances actually move and
   the ICRC-7 owner flips.

That PR is blocked on this one merging.

## Checklist

- [x] New files compile against `icpp-pro` `main` (5.4.1).
- [x] No build-script changes needed (auto-globbed).
- [x] `IC_Call::pending_count()` exposed for diagnostics.
- [x] `IC_Call::clear_pending()` exposed for `pre_upgrade` hooks.
- [x] End-to-end test canister `canister_call` added with `pytest` suite.
- [x] Failure paths return system code so user code can branch on
      "queued vs not queued".
- [x] Reject path forwards IC reject code + message to user handler.
- [ ] (reviewer) Confirm WASM-size delta < 1 KiB on a minimal greet canister.
- [ ] (reviewer) Confirm trampoline name `__icpp_call_*` doesn't collide with
      any reserved icpp internals.
