---
name: icpp-pro-verify-siblings
description: Tiered verification of the sibling repos (icpp-demos, icpp-docs, llama_cpp_canister) after an icpp-pro change — pick the tier from what changed, run the corresponding make targets
disable-model-invocation: false
user-invocable: true
---

# Verify the sibling repos

Read `README-feature-guide.md` Ceremony 4. Pick the tier from what changed in
icpp-pro, then run the target from the icpp-pro repo root:

| What changed                                                 | Run                             |
|--------------------------------------------------------------|---------------------------------|
| Internal only                                                | nothing (icpp-pro tests suffice)|
| Public surface (headers, IC_API, conftest_base, smoketest)   | `make siblings-verify-api`      |
| Build machinery / toolchain / anything wasm-affecting        | `make siblings-verify-api` + `make -C ../llama_cpp_canister docker-build-wasm test-llm-wasm-prebuilt` |
| Release                                                      | `make siblings-verify-full`     |

What the targets do:

- `siblings-verify-api` = `icpp-docs make mkdocs-build` (breaks loudly when a
  `// docs start:` include marker moved) + `icpp-demos make all-tests` +
  `llama_cpp_canister make test-llm-native`. NOTE: the llama native leg needs
  an x86_64 host (ggml `arch/x86/` sources are hard-coded in its icpp.toml);
  on an arm64 Mac it fails at link time — rely on llama's CI for that leg.
- `siblings-verify-full` = the above + llama docker wasm build & prebuilt
  pytest (heavy: docker + models; used at release).

Also useful:

- `make check-sibling-pins` — version & linter pins in sync across all repos.
- Sibling deps into the dev env: `make install-python-w-demos`,
  `make install-python-w-llama_cpp_canister`.
- Sibling layout & override: `SIBLING_ICPP_DEMOS`, `SIBLING_ICPP_DOCS`,
  `SIBLING_LLAMA_CPP_CANISTER` make variables (default `../<repo>`).

Identities are per-repo (`icpp-demos-testing`, `llama-cpp-testing`,
`icpp-pro-testing`) and auto-created by each Makefile; the machine-wide active
identity is never read or written.
