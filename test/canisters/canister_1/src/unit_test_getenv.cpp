// Unit tests for getenv()/environ handling in an ICP canister.
//
// Regression guard for icpp-pro's
//   src/icpp/ic/wasi_sdk_traps/__wasilibc_initialize_environ.c
//
// getenv() reads the canister's environment variables (test_env_vars.cpp). It
// must return NULL for every name that is not set -- and, crucially, MUST NOT
// trap. Also not in a static constructor, which runs in canister_start, where
// the ic0.env_var_* calls trap: there the environment is empty. A previous bug left __wasilibc_environ
// at its (char**)-1 sentinel with no-op ensure/initialize functions, so the
// FIRST getenv() call dereferenced address 0xffffffff -> "heap out of bounds"
// (IC0502). This surfaced e.g. in llama.cpp's tty_can_use_colors() during
// argument parsing.
//
// NOTE on coverage: native/MockIC builds do NOT compile ic/wasi_sdk_traps; they
// use the host libc getenv (which works). So on native this test only verifies
// the invented, never-set name returns NULL. The REAL regression is caught by
// the wasm build (test/test_apis.py::test__getenv), where the icpp override is
// what actually runs -- there, before the fix, this endpoint traps.

#include "unit_test_getenv.h"

#include <cstdlib>

// Runs in canister_start on wasm. ICPP_TEST_GREETING is set in icp.yaml, but
// not visible here; a getenv() that called ic0.env_var_* would trap, and the
// canister would fail to install.
static const bool g_getenv_null_in_start =
    getenv("ICPP_TEST_GREETING") == nullptr;

int unit_test_getenv() {
  int failures = 0;

  if (!g_getenv_null_in_start) {
    failures += 1;
  }

  // Exercise getenv() on common names too. In a canister these return NULL
  // unless set in the canister settings; on the host they may be set. We do NOT assert their
  // value -- the point is that CALLING getenv() must not trap. Before the fix
  // each of these faulted on wasm at the very first call.
  const char *const probe_names[] = {"PATH",     "HOME", "TERM",
                                     "NO_COLOR", "LANG", "PWD"};
  for (const char *name : probe_names) {
    volatile const char *value = getenv(name);
    (void)value;
  }

  // This name is never set on the host or in a canister, so getenv() must
  // return NULL on BOTH native and wasm. Explicitly unset it first so the
  // assertion is deterministic on native (where getenv reads the real host
  // environment) regardless of the developer's shell -- and, as a bonus, this
  // exercises unsetenv() against the (fixed) empty environ on wasm.
  const char *const kUnset = "ICPP_DEFINITELY_UNSET_ENVIRONMENT_VARIABLE_9Z7Q";
  unsetenv(kUnset);
  if (getenv(kUnset) != nullptr) {
    failures += 1;
  }

  // Repeated calls must stay stable (no crash, still NULL).
  for (int i = 0; i < 3; ++i) {
    if (getenv(kUnset) != nullptr) {
      failures += 1;
    }
  }

  return failures;
}
