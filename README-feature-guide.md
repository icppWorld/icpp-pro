# icpp-pro feature development guide

The ceremonies for developing icpp-pro features across the whole repo family.
Any new contributor — human or AI agent — follows this guide for planning,
implementation, testing, and release. Setup lives in
[README-contributors-guide.md](README-contributors-guide.md); the release
mechanics live in [README-release-guide.md](README-release-guide.md).

Claude Code sessions: the skills `icpp-pro-new-feature`,
`icpp-pro-verify-siblings`, and `icpp-pro-release` (in `.claude/skills/`) wrap
the ceremonies below.

## The repo family

All repos are cloned as siblings, per the contributors guide:

| Repo                 | What it is                            | Couples to icpp-pro via                                                     |
|----------------------|---------------------------------------|-----------------------------------------------------------------------------|
| `icpp-pro`           | The CDK (this repo)                   | —                                                                            |
| `icpp-pro/icpp-candid` | C++ Candid library, nested clone    | Same version number, always; `pyproject.toml` pin `icpp-candid>=X.Y.Z`       |
| `icpp-demos`         | Example canisters                     | PyPI `icpp-pro>=X.Y.Z` in `requirements.txt`; test identity `icpp-demos-testing` |
| `icpp-docs`          | MkDocs site → https://docs.icpp.world | `{%include%}`s icpp-pro & icpp-demos **source files by relative path**, anchored on `// docs start:` / `// docs end:` markers |
| `llama_cpp_canister` | llama.cpp as a C++ canister           | PyPI `icpp-pro==X.Y.Z` in `requirements.txt` AND `docker/docker-compose.yml`; wasm-hash sensitive; test identity `llama-cpp-testing` |

`make check-sibling-pins` verifies the version and linter pins across all of
them — run it whenever in doubt.

## The backward-compatibility rule

**Additive only.** A canister built with the previous icpp-pro release must be
upgradable as-is by a build from the next release: no candid interface breaks,
no data-structure migration block, no source changes. Concretely:

- The C++ API only grows: existing public signatures are never changed or
  removed; extend with new methods/overloads.
- The candid wire encoding of existing types never changes.
- Deployed-canister state survives an upgrade: no re-ordering or re-sizing of
  persisted structures, no changes to the stable-memory / wasi-polyfill layout.

Breaking is the rare exception: it requires explicit maintainer sign-off in
the plan, a documented migration path, and a `migrating-to-X.Y.Z.md` page in
icpp-docs. The gate that enforces the rule is `make upgrade-test` (Ceremony 3).

## Branching & PRs

Feature work never happens on `main`. One feature = one branch name, and that
**exact same branch name is used in every repo the feature touches** — that is
what makes cross-repo work trackable and the PRs easy to relate.

- Name: `feature/<short-kebab-slug>`, e.g. `feature/cycles128`. Pick it once,
  in the plan (Ceremony 1).
- Create it in icpp-pro when implementation starts:
  `git checkout -b feature/<slug>`. Create the same branch in a sibling repo
  lazily — at the moment that repo actually needs a change — never
  preemptively in repos the feature does not touch.
- One PR per repo, all from the same branch name. Cross-reference the icpp-pro
  PR from each sibling PR.
- Merge order: **icpp-pro first** (the siblings build against it), then
  icpp-demos, then icpp-docs (its includes read the other two), then
  llama_cpp_canister.
- Exception: the release ceremony pushes the version-bump commit directly to
  `main`, as prescribed by README-release-guide.md.

## Ceremony 1 — Plan

1. Start from the roadmap: `todo/ic0-api-inventory.html` tracks the IC0-parity
   project (coverage, sequencing, what is done). Check it before and after.
2. Explore first: how does Motoko do it (`../dfinity/motoko`), what does the
   replica accept (`../dfinity/ic/rs/embedders/src/wasm_utils/validation.rs`),
   what exists in icpp-pro already.
3. Write the plan with three mandatory sections:
   - **Definition of Done** (see Ceremony 2 — all five layers).
   - **Compatibility assessment**: classify the change additive vs breaking
     for (a) the C++ API, (b) the candid wire encoding, (c) deployed-canister
     state. The default answer to "can existing canisters upgrade as-is?"
     must be **yes**; anything else needs explicit sign-off.
   - **Branch name**: the `feature/<slug>` used in every repo the feature
     touches (see Branching & PRs), plus the list of repos expected to change.

## Ceremony 2 — Implement

Start on the branch: `git checkout -b feature/<slug>` in icpp-pro; the same
name in each sibling repo at the moment it needs a change.

Definition of Done for a new IC capability — five layers, always together:

1. **ic0 binding** — `src/icpp/ic/ic0/ic0.h`: `ic0_<import_name>`, `uint32_t`
   for pointers (wasm32), `WASM_SYMBOL_IMPORTED("ic0", "<import_name>")`.
2. **Mock** — `src/icpp/ic/ic0mock/ic0.h` (same name, `uintptr_t` for
   pointers — 64-bit native) + a real implementation in `ic0mock/ic0.cpp`
   backed by `MockIC`, not a print-and-return-0 stub.
3. **IC_API wrapper** — `src/icpp/ic/icapi/ic_api.{h,cpp}`, wrapped in
   `// docs start: <name>` / `// docs end: <name>` markers (icpp-docs includes
   these line ranges — moving or renaming a marker breaks the docs build).
4. **Test canister** — coverage in `test/canisters/` (extend an existing
   canister when it fits; a new `canister_<name>` when the capability needs
   its own lifecycle).
5. **Docs & demos** — Ceremony 5.

Compatibility while coding: add, never change or remove, public signatures;
follow the existing header conventions; C++17-compatible.

## Ceremony 3 — Test (icpp-pro)

```bash
conda activate icpp-pro
make all-static                  # clang-format + black + pylint + mypy
# targeted, for the canisters you touched:
cd test/canisters/<canister> && icpp build-native && ./build-native/mockic.exe
cd test/canisters/<canister> && icpp build-wasm
# full suite before merge:
make all-tests
```

CI (mac + ubuntu matrix, python 3.11-3.14) must be green.

**Upgrade test** — required when the change can affect deployed state or the
wire format, and always at release:

```bash
make upgrade-test
```

This deploys `../icpp-demos/canisters/files` built with the **released**
icpp-pro from PyPI, runs its full pytest (which writes state), rebuilds the
same source with the **dev** tree, upgrades the canister in place, and runs
the `run_after_upgrade`-marked tests. Options:
`python -m scripts.upgrade_test --canister-dir <dir> --released-version X.Y.Z`.

## Ceremony 4 — Verify siblings (tiered)

| What changed                                                | What to run                     |
|-------------------------------------------------------------|---------------------------------|
| Internal only (no public header / API / build change)       | nothing beyond Ceremony 3       |
| Public surface: headers, `IC_API`, `conftest_base`, `smoketest` | `make siblings-verify-api`  |
| Build machinery, toolchain versions, anything wasm-affecting | `make siblings-verify-api` **plus** `make -C ../llama_cpp_canister docker-build-wasm test-llm-wasm-prebuilt` |
| Release                                                      | `make siblings-verify-full`     |

`make siblings-verify-api` = icpp-docs `mkdocs-build` (cheap — a moved include
marker fails loudly) + icpp-demos `all-tests` + llama_cpp_canister
`test-llm-native`. `make siblings-verify-full` adds the wasm-hash-sensitive
llama docker build + prebuilt-wasm pytest (heavy: docker + models).

Host caveat: llama's `test-llm-native` needs an x86_64 host (its `icpp.toml`
hard-codes ggml `arch/x86/` sources — see the note there). On an arm64 Mac
that leg fails at link time; rely on llama's CI (which runs it) or its docker
path for that repo, and treat docs + demos as the local tier.

Each sibling manages its own test identity (`icpp-demos-testing`,
`llama-cpp-testing`); nothing ever touches the machine-wide active identity.
Sibling deps install into the dev env with `make install-python-w-demos` /
`make install-python-w-llama_cpp_canister`.

## Ceremony 5 — Document

For every user-facing capability:

1. `icpp-docs/docs/api-reference.md`: add a `####` section with a
   `{%include "../../icpp-pro/src/icpp/ic/icapi/ic_api.h" start=... end=...%}`
   snippet keyed on the `// docs start:` markers from Ceremony 2.
2. `icpp-demos/canisters/api_reference/src/`: add a demo function when it
   teaches something (the docs include these too).
3. `icpp-docs/docs/release-notes.md`: one line per capability.
4. Verify: `make -C ../icpp-docs mkdocs-build` (or `mkdocs-serve`, port 8100).

## Ceremony 6 — Track & commit

1. Update `todo/ic0-api-inventory.html`: coverage counts, callouts, sequencing
   check-offs — and republish its artifact (same URL, listed in
   `todo/artifacts.md`).
2. New shareable pages go in `todo/` and get a row in `todo/artifacts.md`.
3. Commits: single-line message, no description body, no `Co-Authored-By`
   trailers, never `--no-verify`. One commit per repo, on the shared
   `feature/<slug>` branch (see Branching & PRs). Push only when the
   maintainer says so.
4. PRs: one per changed repo from the same branch name, cross-referencing the
   icpp-pro PR; merge icpp-pro first, llama_cpp_canister last.

## Ceremony 7 — Release

Follow [README-release-guide.md](README-release-guide.md) end to end. The
gates added by this guide:

- `make check-sibling-pins` — before and after the sibling pin bumps.
- `make upgrade-test` — the released → release-candidate upgrade must be clean.
- `make siblings-verify-full` — the full trio, mandatory.
- Sibling pin bumps (exact locations are in the release guide follow-ups):
  icpp-demos `requirements.txt`, llama_cpp_canister `requirements.txt` +
  `docker/docker-compose.yml` (two spots), icpp-docs version labels.
