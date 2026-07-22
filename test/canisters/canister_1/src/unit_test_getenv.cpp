// Unit tests for getenv()/environ handling in an ICP canister.
//
// Regression guard for icpp-pro's
//   src/icpp/ic/wasi_sdk_traps/__wasilibc_initialize_environ.c
//
// A canister has no OS environment, so getenv() must return NULL for every
// name -- and, crucially, MUST NOT trap. A previous bug left __wasilibc_environ
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

int unit_test_getenv() {
  int failures = 0;

  // Exercise getenv() on common names too. In a canister (empty environment)
  // these all return NULL; on the host they may be set. We do NOT assert their
  // value -- the point is that CALLING getenv() must not trap. Before the fix
  // each of these faulted on wasm at the very first call.
  const char *const probe_names[] = {"PATH",     "HOME", "TERM",
                                     "NO_COLOR", "LANG", "PWD"};
  for (const char *name : probe_names) {
    volatile const char *value = getenv(name);
    (void)value;
  }

  // This name is never set on the host or in a canister, so getenv() must
  // return NULL on BOTH native and wasm.
  const char *const kUnset = "ICPP_DEFINITELY_UNSET_ENVIRONMENT_VARIABLE_9Z7Q";
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
