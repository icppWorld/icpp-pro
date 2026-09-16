---
name: icpp-pro-new-feature
description: The per-feature ceremony loop for icpp-pro — plan with a compatibility assessment, implement all five layers (ic0 binding, mock, IC_API, test canister, docs), test including the released-to-dev upgrade gate, and track progress on the roadmap
disable-model-invocation: false
user-invocable: true
---

# New feature development for icpp-pro

Read `README-feature-guide.md` (repo root) and follow Ceremonies 1-3, 5, 6 in
order. This skill is the checklist; the guide is the source of truth.

## 1. Plan (guide Ceremony 1)

- Check the roadmap first: `todo/ic0-api-inventory.html` (+ its artifact,
  URL in `todo/artifacts.md`).
- Reference implementations: `../dfinity/motoko` (what Motoko does),
  `../dfinity/ic/rs/embedders/src/wasm_utils/validation.rs` (what the replica
  accepts).
- The plan MUST contain a **compatibility assessment**: additive vs breaking
  for the C++ API, the candid wire encoding, and deployed-canister state.
  Default must be additive — a canister built with the previous release
  upgrades as-is. Breaking needs explicit user sign-off + migration plan.
- The plan MUST name the **branch**: `feature/<short-kebab-slug>`, used with
  the EXACT same name in every repo the feature touches, plus the expected
  repo list.

## 2. Implement (guide Ceremony 2) — five layers, always together

First: `git checkout -b feature/<slug>` in icpp-pro — never work on `main`.
Create the same branch in a sibling repo only at the moment it needs a change.

1. ic0 binding: `src/icpp/ic/ic0/ic0.h` (`ic0_<name>`, `uint32_t` pointers).
2. Mock: `src/icpp/ic/ic0mock/ic0.h` (`uintptr_t` pointers) + real MockIC
   implementation in `ic0mock/ic0.cpp` — not a return-0 stub.
3. IC_API wrapper in `src/icpp/ic/icapi/` with `// docs start:`/`// docs end:`
   markers (icpp-docs includes these ranges — never move/rename carelessly).
4. Test-canister coverage in `test/canisters/`.
5. Docs & demos (guide Ceremony 5).

## 3. Test (guide Ceremony 3)

```bash
make all-static
cd test/canisters/<touched> && icpp build-native && ./build-native/mockic.exe
cd test/canisters/<touched> && icpp build-wasm
make all-tests          # before merge
make upgrade-test       # when deployed state / wire format could be affected
```

Then run the sibling tier that matches the change — see the
`icpp-pro-verify-siblings` skill.

## 4. Track & commit (guide Ceremony 6)

- Update `todo/ic0-api-inventory.html` (counts, check-offs) and republish its
  artifact to the SAME url; log new artifacts in `todo/artifacts.md`.
- Commit per repo on the shared `feature/<slug>` branch: single-line message,
  no trailers, no `--no-verify`. Push only when the user asks.
- PRs: one per changed repo from the same branch name, cross-referencing the
  icpp-pro PR. Merge order: icpp-pro → icpp-demos → icpp-docs →
  llama_cpp_canister.
- Every PR must receive a MANUAL approval from the maintainer before merging.
  Never merge a PR yourself — ask the user for the approval and wait for it.
