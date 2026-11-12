---
name: icpp-pro-release
description: Release ceremony for icpp-pro + icpp-candid — version bump, upgrade gate, full sibling verification, the tag-triggered trusted-publishing release, and the sibling version-pin bumps that follow
disable-model-invocation: false
user-invocable: true
---

# Release icpp-pro

Follow `README-release-guide.md` (repo root) step by step — it is the source
of truth. The gates and follow-ups to not skip:

## Before the release tag

1. `make check-sibling-pins` — see what will need bumping.
2. Version bump, same version in both repos: `icpp-pro/src/icpp/version.py`
   and `icpp-candid/src/icpp_candid/version.py`, plus the
   `icpp-candid==X.Y.Z` pin in icpp-pro's `pyproject.toml`. Push icpp-candid
   first (icpp-pro CI needs it).
3. `make upgrade-test` — a canister deployed with the previous PyPI release
   must upgrade in place to the release candidate, state intact. This gate is
   mandatory at release.
4. `make siblings-verify-api` — docs, demos and llama native against the dev
   install. NOT `siblings-verify-full`: before the release, llama's docker
   build can only install the previous icpp-pro from PyPI.
5. The release guide's own testing matrix (python 3.11-3.14, wheel installs,
   demo scripts with the grep-based log verification, the definitive pytest).
6. The deep reviews of the release, on a review-only PR per repo of
   everything since the previous tag (the guide's "Deep reviews of the
   release" has the commands): ask the user to run `/code-review ultra` on
   each (billed; only the user can start it), and run `/claude-security`
   "Scan changes" at high effort. Fix their findings through PRs that merge
   before the fuzz run, since that run must be on the commits to tag. After
   the release, close the review PRs and delete their `release-base/` branches,
   never with `gh pr close --delete-branch` (the head is `main`).
7. `gh workflow run fuzz.yml --ref main -f seconds=3600` — an hour per target
   of the Candid decoder fuzzers, about 3.5 hours in GitHub Actions (CI runs
   only a minute). A crash blocks the release: fix it in icpp-candid and keep
   the input, from the run's `fuzz-crashes` artifact, in
   `test/fuzz/regressions/`. Put the run length and the result in the release
   notes.

## Publish (tag-triggered, CI-only)

**Prerequisite:** the manually triggered `fuzz` workflow (step 7) has passed
with `seconds=3600` on the exact icpp-pro and icpp-candid commits to be tagged.
Do not tag before that; the commits are in the run's `versions` step.

Publishing happens ONLY by pushing a `vX.Y.Z` tag, which runs that repo's
`.github/workflows/release.yml` and uploads to PyPI with trusted publishing
(OIDC). No API token, no `.pypirc`, never an upload from a laptop. Never try
to publish by hand.

- Tag **icpp-candid first**, wait for it to be on PyPI
  (`pip download icpp-candid==X.Y.Z --no-deps`), then tag icpp-pro —
  icpp-pro's workflow refuses to publish until that version resolves.
- The tag must point at a commit whose `version.py` already says X.Y.Z; both
  workflows assert this.
- Monitor each run to completion: **a red leg means nothing was published.**

## After the release lands (PyPI)

Bump the sibling pins to the new version, then `make check-sibling-pins`
until green:

- `icpp-demos/requirements.txt` — `icpp-pro>=X.Y.Z`
- `llama_cpp_canister/requirements.txt` — `icpp-pro==X.Y.Z` AND
  `llama_cpp_canister/docker/docker-compose.yml` (`icpp: &icpp "X.Y.Z"` and
  the `name: &base_name ...icpp-X.Y.Z` literal). The wasm hash changes — the
  llama repo has its own release process
  (`llama_cpp_canister/.claude/skills/llama_cpp_canister-release`).
  With the pin bumped, run `make siblings-verify-full` — the llama docker
  wasm leg against the published release (hours; plan for it).
- `icpp-docs`: `mkdocs.yml` nav version label, `docs/index.md`,
  `docs/release-notes.md`; then `make -C ../icpp-docs site-build` and
  `make -C ../icpp-docs icp-deploy`.

Plus the release guide's follow-ups: redeploy the greet canister to the IC,
redeploy the icpp-demos api_reference canister, OpenChat announcement.
