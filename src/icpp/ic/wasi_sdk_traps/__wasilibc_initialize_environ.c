// Stub all methods of wasi-libc/libc-bottom-half/sources/__wasilibc_initialize_environ.c

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

// See the comments in libc-environ.h.
void __wasilibc_ensure_environ(void) {
  // Do Nothing
}

/// Avoid dynamic allocation for the case where there are no environment
/// variables, but we still need a non-NULL pointer to an (empty) array.
static char *empty_environ[1] = { NULL };

// See the comments in libc-environ.h.
void __wasilibc_initialize_environ(void) {
  // Do Nothing
}

// See the comments in libc-environ.h.
void __wasilibc_deinitialize_environ(void) {
  // Do Nothing
}

// See the comments in libc-environ.h.
__attribute__((weak))
void __wasilibc_maybe_reinitialize_environ_eagerly(void) {
    // This version does nothing. It may be overridden by a version which does
    // something if `environ` is used.
}
