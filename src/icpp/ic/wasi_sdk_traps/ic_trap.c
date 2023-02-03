// trap function for use in C routines

#include <sys/uio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include "ic0.h"
#include "ic_trap.h"

#include <limits.h>
#include <errno.h>
#include <stdarg.h>

void ic_trap(const char *str) {
  size_t len = strlen(str);
  ic0_trap((uint32_t)(str), (uint32_t)(len));
}