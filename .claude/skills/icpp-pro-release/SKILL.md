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
   `icpp-candid>=X.Y.Z` pin in icpp-pro's `pyproject.toml`. Push icpp-candid
   first (icpp-pro CI needs it).
3. `make upgrade-test` — a canister deployed with the previous PyPI release
   must upgrade in place to the release candidate, state intact. This gate is
   mandatory at release.
4. `make siblings-verify-full` — the full trio including the llama docker
   wasm build (hours; plan for it).
5. The release guide's own testing matrix (python 3.11-3.14, wheel installs,
   demo scripts with the grep-based log verification, the definitive pytest).

## Publish (tag-triggered, CI-only)

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
- `icpp-docs`: `mkdocs.yml` nav version label, `docs/index.md`,
  `docs/release-notes.md`; then `make -C ../icpp-docs site-build` and
  `make -C ../icpp-docs icp-deploy`.

Plus the release guide's follow-ups: redeploy the greet canister to the IC,
redeploy the icpp-demos api_reference canister, OpenChat announcement.
