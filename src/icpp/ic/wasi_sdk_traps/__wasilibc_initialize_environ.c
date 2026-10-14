// Stub all methods of wasi-libc/libc-bottom-half/sources/__wasilibc_initialize_environ.c
//
// A canister has no OS environment. Rather than pull in wasi-libc's real
// environ init (which calls the WASI environ_sizes_get/environ_get imports),
// we build `environ` from the canister's environment variables, read with the
// ic0.env_var_* system API. getenv() etc. see a valid, NULL-terminated
// `environ` array, and return NULL for every name that is not set.
//
// The ic0.env_var_* calls trap in canister_start, which is where the C++
// static constructors run (wasi2ic turns _initialize into the wasm start
// function). So environ is only built while an entry point other than
// canister_start is executing - recorded by IC_API through
// icpp_environ_set_available() - and is empty otherwise, as it was before
// icpp-pro 6.5.0. It is rebuilt for every entry point, because the
// environment variables can change between messages (update_settings).
// Changes made with setenv() / putenv() therefore last for one message.
//
// IMPORTANT (bug fixed 2026-07): the previous version left every function as a
// no-op, so `__wasilibc_environ` stayed at its (char**)-1 sentinel. getenv()
// calls __wasilibc_ensure_environ() (a no-op) and then dereferenced -1
// (address 0xffffffff) -> "heap out of bounds" trap (IC0502) on the very first
// getenv() call. The functions now point __wasilibc_environ at a valid array.

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include "ic0.h"
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

// Whether ic0.env_var_* may be called: an entry point other than
// canister_start is executing.
static int environ_available = 0;

// The array built from the environment variables: the NULL-terminated pointer
// array, followed by the "NAME=VALUE" strings, in one allocation.
static char **built_environ = NULL;

// Builds the "NAME=VALUE" array from ic0.env_var_*, or returns NULL when out
// of memory.
static char **build_environ(void) {
  uint32_t count = ic0_env_var_count();
  size_t strings_size = 0;
  for (uint32_t i = 0; i < count; ++i) {
    uint32_t name_size = ic0_env_var_name_size(i);
    char *name = malloc(name_size);
    if (name == NULL) return NULL;
    ic0_env_var_name_copy(i, (uint32_t)(uintptr_t)name, 0, name_size);
    strings_size += name_size +
                    ic0_env_var_value_size((uint32_t)(uintptr_t)name,
                                           name_size) +
                    2; // '=' and '\0'
    free(name);
  }

  size_t array_size = (count + 1) * sizeof(char *);
  char **env = malloc(array_size + strings_size);
  if (env == NULL) return NULL;

  char *p = (char *)env + array_size;
  for (uint32_t i = 0; i < count; ++i) {
    uint32_t name_size = ic0_env_var_name_size(i);
    ic0_env_var_name_copy(i, (uint32_t)(uintptr_t)p, 0, name_size);
    uint32_t value_size =
        ic0_env_var_value_size((uint32_t)(uintptr_t)p, name_size);
    p[name_size] = '=';
    ic0_env_var_value_copy((uint32_t)(uintptr_t)p, name_size,
                           (uint32_t)(uintptr_t)(p + name_size + 1), 0,
                           value_size);
    p[name_size + 1 + value_size] = '\0';
    env[i] = p;
    p += name_size + value_size + 2;
  }
  env[count] = NULL;
  return env;
}

// See the comments in libc-environ.h.
void __wasilibc_initialize_environ(void) {
  free(built_environ);
  built_environ = environ_available ? build_environ() : NULL;
  __wasilibc_environ = built_environ != NULL ? built_environ : empty_environ;
}

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
void __wasilibc_deinitialize_environ(void) {
  free(built_environ);
  built_environ = NULL;
  __wasilibc_environ = (char **)-1;
}

// See the comments in libc-environ.h.
// This version does nothing, as in wasi-libc. When the program references
// `environ` directly rather than via getenv(), wasi-libc's environ.c is
// linked in: it overrides this with a version that initializes environ, and
// initializes it eagerly in a constructor.
__attribute__((weak)) void __wasilibc_maybe_reinitialize_environ_eagerly(void) {
}

// Called by IC_API when an entry point starts (available: not canister_start)
// and when it ends (0). Resets environ, so the next getenv() rebuilds it for
// the new state - or right away, for a program that uses `environ` directly.
void icpp_environ_set_available(int available) {
  environ_available = available;
  __wasilibc_deinitialize_environ();
  __wasilibc_maybe_reinitialize_environ_eagerly();
}
