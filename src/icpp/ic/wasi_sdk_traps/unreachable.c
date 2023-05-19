// Modied from: https://github.com/ICPorts-labs/chico/blob/main/src/unreachable.c

#include <dirent.h>
#include <sys/socket.h>

#include "ic_trap.h"
#include <stdint.h>
#include <string.h>
#include <sys/uio.h>
#include <unistd.h>

#include <errno.h>
#include <limits.h>
#include <stdarg.h>

// All functions that are unsupported in Wasm

void *dlopen(const char *file, int mode) {
  ic_trap("Your code is calling a stubbed out function:  dlopen");
  return 0;
}

char *dlerror(void) {
  ic_trap("Your code is calling a stubbed out function:  dlerror");
  return 0;
}

void *dlsym(void *restrict p, const char *restrict s) {
  ic_trap("Your code is calling a stubbed out function:  dlsym");
  return 0;
}

int dlclose(void *handle) {
  ic_trap("Your code is calling a stubbed out function:  dlclose");
  return 0;
}

int __muloti4(int a, int b, int *overflow) {
  ic_trap("Your code is calling a stubbed out function:  __muloti4");
  return 1;
}

long sysconf(int name) {
  ic_trap("Your code is calling a stubbed out function:  sysconf");
  return 1;
}

int close(int fd) {
  ic_trap("Your code is calling a stubbed out function:  close");
  return 1;
}

// is in posix.c
// int access(const char *pathname, int mode){
//   ic_trap("Your code is calling a stubbed out function:  access");
//   return 1;
// }

int stat64(const char *pathname, int mode) {
  ic_trap("Your code is calling a stubbed out function:  stat64");
  return 1;
}

int fstat64(const char *pathname, int mode) {
  ic_trap("Your code is calling a stubbed out function:  fstat64");
  return 1;
}

int fchmod(const char *pathname, int mode) {
  ic_trap("Your code is calling a stubbed out function:  fchmod");
  return 1;
}

// is in posix.c
// int mkdir(const char *pathname, int mode){
//   ic_trap("Your code is calling a stubbed out function:  mkdir");
//   return 1;
// }

int munmap(const char *pathname, int mode) {
  ic_trap("Your code is calling a stubbed out function:  munmap");
  return 1;
}

int lstat64(const char *pathname, int mode) {
  ic_trap("Your code is calling a stubbed out function:  lstat64");
  return 1;
}

int utimes(const char *pathname, int mode) {
  ic_trap("Your code is calling a stubbed out function:  utimes");
  return 1;
}

int gettimeofday(const char *pathname, int mode) {
  ic_trap("Your code is calling a stubbed out function:  gettimeofday");
  return 1;
}

int nanosleep(const char *pathname, int mode) {
  ic_trap("Your code is calling a stubbed out function:  nanosleep");
  return 1;
}

int __munmap(const char *pathname, int mode) {
  ic_trap("Your code is calling a stubbed out function:  __mumap");
  return 1;
}

int __map_file(const char *pathname, int mode) {
  ic_trap("Your code is calling a stubbed out function:  __map_file");
  return 1;
}

// is in posix.c
// int unlink(const char *pathname) {
//    ic_trap("Your code is calling a stubbed out function:  unlink");
//    return 1;
// }

// is in posix.c
// int rmdir(const char *pathname) {
//    ic_trap("Your code is calling a stubbed out function:  rmdir");
//    return 1;
// }

int fdatasync(int fd) {
  ic_trap("Your code is calling a stubbed out function:  fdatasync");
  return 1;
}

int __expand_heap(const char *pathname) {
  ic_trap("Your code is calling a stubbed out function:  __expand_heap");
  return 1;
}

ssize_t read(int fd, void *buf, size_t count) {
  ic_trap("Your code is calling a stubbed out function:  read");
  return 1;
}

void __lock(volatile int *l) {
  ic_trap("Your code is calling a stubbed out function:  __lock");
}

void __unlock(volatile int *l) {
  ic_trap("Your code is calling a stubbed out function:  __unlock");
}

int *___errno_location(void) {
  ic_trap("Your code is calling a stubbed out function:  __errno_location");
  return 0;
}

int *__errno_location(void) {
  ic_trap("Your code is calling a stubbed out function:  __errno_location");
  return 0;
}

uid_t geteuid(void) {
  ic_trap("Your code is calling a stubbed out function:  getetuid");
  return 1;
}

char *getcwd(char *buf, size_t size) {
  ic_trap("Your code is calling a stubbed out function:  getcwd");
  return 0;
}

ssize_t write(int fd, const void *buf, size_t count) {
  ic_trap("Your code is calling a stubbed out function:  write");
  return 1;
}

ssize_t writev(int fildes, const struct iovec *iov, int iovcnt) {
  ic_trap("Your code is calling a stubbed out function:  writev");
  return 1;
}

ssize_t fcntl64(int fd, const void *buf, size_t count) {
  ic_trap("Your code is calling a stubbed out function:  fcntl64");
  return 1;
}

int fchown(int fd, uid_t uid, gid_t gid) {
  ic_trap("Your code is calling a stubbed out function:  fchown");
  return 1;
}

// is in posix.c
// ssize_t readlink(const char *restrict path, char *restrict buf, size_t bufsize)
// {
//   ic_trap("Your code is calling a stubbed out function:  readlink");
//   return 1;
// }

ssize_t open64(int fd, const void *buf, size_t count) {
  ic_trap("Your code is calling a stubbed out function:  open64");
  return 1;
}

pid_t getpid(void) {
  ic_trap("Your code is calling a stubbed out function:  getpid");
  return 1;
}

off_t __lseek(int fildes, off_t offset, int whence) {
  ic_trap("Your code is calling a stubbed out function:  lseek");
  return 1;
}

off_t lseek64(int fd, off_t offset, int whence) {
  ic_trap("Your code is calling a stubbed out function:  lseek64");
  return 1;
}

/*
This gives a warning during loading:

wasm-ld: warning: function signature mismatch: __multi3
>>> defined as (i32, i64, i64, i64, i64) -> void in unreachable.o
>>> defined as (i64, i64) -> i64 in lto.tmp
*/
//long long __multi3 (long long a , long long b ) {
//ic_trap("Your code is calling a stubbed out function:  __multi3");
//return 1;
//}

long double __multf3(long double a, long double b) {
  ic_trap("Your code is calling a stubbed out function:  __multf3");
  return 1;
}

long double __subtf3(long double a, long double b) {
  ic_trap("Your code is calling a stubbed out function:  __subtf3");
  return 1;
}

long double __divtf3(long double a, long double b) {
  ic_trap("Your code is calling a stubbed out function:  __divtf3");
  return 1;
}

long double __addtf3(long double a, long double b) {
  ic_trap("Your code is calling a stubbed out function:  __addtf3");
  return 1;
}

void *mmap64(void *start, size_t len, int prot, int flags, int fd, off_t off) {
  ic_trap("Your code is calling a stubbed out function:  mmap64");
  return 0;
}

int __letf2(long double a, long double b) {
  ic_trap("Your code is calling a stubbed out function:  __letf2");
  return 1;
}

int __getf2(long double a, long double b) {
  ic_trap("Your code is calling a stubbed out function:  __getf2");
  return 1;
}

int __lttf2(long double a, long double b) {
  ic_trap("Your code is calling a stubbed out function:  __lttf2");
  return 1;
}

int __gttf2(long double a, long double b) {
  ic_trap("Your code is calling a stubbed out function:  __gttf2");
  return 1;
}

void __floatditf(long a, long long b) {
  ic_trap("Your code is calling a stubbed out function:  __floatditf");
}

void __floatsitf(int i, int j) {
  ic_trap("Your code is calling a stubbed out function:  __floatsitf");
}

int __fixtfsi(long double a) {
  ic_trap("Your code is calling a stubbed out function:  __fixtfsi");
  return 1;
}

double __trunctfdf2(long double a) {
  ic_trap("Your code is calling a stubbed out function:  __trunctfdf2");
  return 1;
}

long double __extenddftf2(double a) {
  ic_trap("Your code is calling a stubbed out function:  __extenddftf2 ");
  return 1;
}

int posix_fallocate64(int fd, off_t base, off_t len) {
  ic_trap("Your code is calling a stubbed out function:  posix_fallocate64");
  return 1;
}

char *strerror(int e) {
  ic_trap("Your code is calling a stubbed out function:  strerror");
  return 0;
}

off_t ftruncate64(int fd, off_t offset) {
  ic_trap("Your code is calling a stubbed out function:  ftruncate64");
  return 1;
}

// do NOT stub this, it is used by std::to_string(float x)
// int vsnprintf(char *restrict s, size_t n, const char *restrict fmt, va_list ap) {
//   ic_trap("Your code is calling a stubbed out function:  vsnprintf");
//   return 1;
// }

int fcntl(int fildes, int cmd, ...) {
  ic_trap("Your code is calling a stubbed out function:  fcntl");
  return 1;
}

int __wasilibc_nocwd_openat_nomode(int fd, const char *path, int oflag) {
  ic_trap(
      "Your code is calling a stubbed out function:  __wasilibc_nocwd_openat_nomode");
  return 1;
}

int ioctl(int fildes, int request, ...) {
  ic_trap("Your code is calling a stubbed out function:  ioctl");
  return 1;
}

int getsockopt(int socket, int level, int option_name,
               void *restrict option_value, socklen_t *restrict option_len) {
  ic_trap("Your code is calling a stubbed out function:  getsockopt");
  return 1;
}

int __wasilibc_nocwd_faccessat(int fd, const char *path, int amode, int flag) {
  ic_trap(
      "Your code is calling a stubbed out function:  __wasilibc_nocwd_faccessat");
  return 1;
}

ssize_t pread(int fildes, void *buf, size_t nbyte, off_t offset) {
  ic_trap("Your code is calling a stubbed out function:  pread");
  return 1;
}

ssize_t pwrite(int fildes, const void *buf, size_t nbyte, off_t offset) {
  ic_trap("Your code is calling a stubbed out function:  pwrite");
  return 1;
}

int __isatty(int fd) {
  ic_trap("Your code is calling a stubbed out function:  __isatty");
  return 1;
}

DIR *fdopendir(int fd) {
  ic_trap("Your code is calling a stubbed out function:  fdopendir");
  return 0;
}

struct dirent *readdir(DIR *dirp) {
  ic_trap("Your code is calling a stubbed out function:  readdir");
  return 0;
}

int __wasilibc_nocwd_scandirat(int dirfd, const char *dir,
                               struct dirent ***namelist,
                               int (*sel)(const struct dirent *),
                               int (*compar)(const struct dirent **,
                                             const struct dirent **)) {
  ic_trap(
      "Your code is calling a stubbed out function:  __wasilibc_nocwd_scandirat");
  return 1;
}

ssize_t readv(int fildes, const struct iovec *iov, int iovcnt) {
  ic_trap("Your code is calling a stubbed out function:  readv");
  return 1;
}
