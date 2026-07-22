// Stub all methods of wasi-libc/libc-bottom-half/sources/__wasilibc_initialize_environ.c
//
// A canister has no OS environment. Rather than pull in wasi-libc's real
// environ init (which calls the WASI environ_sizes_get/environ_get imports),
// we present an EMPTY environment: getenv() etc. must see a valid, empty,
// NULL-terminated `environ` array and simply return NULL for every name.
//
// IMPORTANT (bug fixed 2026-07): the previous version left every function as a
// no-op, so `__wasilibc_environ` stayed at its (char**)-1 sentinel. getenv()
// calls __wasilibc_ensure_environ() (a no-op) and then dereferenced -1
// (address 0xffffffff) -> "heap out of bounds" trap (IC0502) on the very first
// getenv() call. The `empty_environ` array below was defined for exactly this
// purpose but was never assigned. The functions now point __wasilibc_environ at
// it, so getenv() safely returns NULL.

#include <unistd.h>
#include <stdlib.h>
#include <sysexits.h>
#include "ic_trap.h"

/// If the program doesn't use `environ`, it'll get this version of
/// `__wasilibc_environ`, which isn't initialized with a constructor function.
/// `getenv` etc. call `__wasilibc_ensure_environ()` before accessing it.
/// Statically-initialize it to an invalid pointer value so that we can
/// detect if it's been explicitly initialized (we can't use `NULL` because
/// `clearenv` sets it to NULL.
char **__wasilibc_environ __attribute__((weak)) = (char **)-1;

/// Avoid dynamic allocation for the case where there are no environment
/// variables, but we still need a non-NULL pointer to an (empty) array.
static char *empty_environ[1] = {NULL};

// See the comments in libc-environ.h.
// A canister always has an empty environment, so "initialize" simply points
// __wasilibc_environ at the empty (NULL-terminated) array.
void __wasilibc_initialize_environ(void) { __wasilibc_environ = empty_environ; }

// See the comments in libc-environ.h.
// getenv() and friends call this before touching __wasilibc_environ. Initialize
// on first use (when still the (char**)-1 sentinel), matching the semantics of
// the real wasi-libc implementation.
void __wasilibc_ensure_environ(void) {
  if (__wasilibc_environ == (char **)-1) {
    __wasilibc_initialize_environ();
  }
}

// See the comments in libc-environ.h.
// Reset to the sentinel so a subsequent __wasilibc_ensure_environ() re-inits.
void __wasilibc_deinitialize_environ(void) { __wasilibc_environ = (char **)-1; }

// See the comments in libc-environ.h.
// Pulled in (over the weak stub) when the program references `environ`
// directly rather than via getenv(). Point it at the empty array so those
// direct accesses are safe too.
__attribute__((weak)) void __wasilibc_maybe_reinitialize_environ_eagerly(void) {
  __wasilibc_initialize_environ();
}
