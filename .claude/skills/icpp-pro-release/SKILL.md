---
name: icpp-pro-release
description: Release ceremony for icpp-pro + icpp-candid — version bump, upgrade gate, full sibling verification, PyPI upload, and the sibling version-pin bumps that follow
disable-model-invocation: false
user-invocable: true
---

# Release icpp-pro

Follow `README-release-guide.md` (repo root) step by step — it is the source
of truth. The gates and follow-ups to not skip:

## Before upload

1. `make check-sibling-pins` — see what will need bumping.
2. Version bump: `src/icpp/version.py` in BOTH icpp-pro and icpp-candid (they
   always share the same version), plus the `icpp-candid>=X.Y.Z` pin in
   icpp-pro's `pyproject.toml`. Push icpp-candid first (icpp-pro CI needs it).
3. `make upgrade-test` — a canister deployed with the previous PyPI release
   must upgrade in place to the release candidate, state intact. This gate is
   mandatory at release.
4. `make siblings-verify-full` — the full trio including the llama docker
   wasm build (hours; plan for it).
5. The release guide's own testing matrix (python 3.11-3.14, wheel installs,
   demo scripts with the grep-based log verification, the definitive pytest).

## After upload (PyPI)

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
