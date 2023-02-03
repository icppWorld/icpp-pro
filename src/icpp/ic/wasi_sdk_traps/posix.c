// Stub all methods of wasi-libc/libc-bottom-half/sources/posix.c

//! POSIX-like functions supporting absolute path arguments, implemented in
//! terms of `__wasilibc_find_relpath` and `*at`-style functions.

#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>

// --- 
#include <sys/uio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include "ic_trap.h"

// ---
#include <limits.h>
#include <errno.h>
#include <stdarg.h>

static int find_relpath2(
    const char *path,
    char **relative,
    size_t *relative_len
) {
  ic_trap("calling find_relpath2");
  return 0;
}

static int find_relpath(const char *path, char **relative) {
  ic_trap("calling find_relpath");
  return 0;
}

static int find_relpath_alt(const char *path, char **relative) {
  ic_trap("calling find_relpath_alt");
  return 0;
}

int open(const char *path, int oflag, ...) {
  ic_trap("calling open");
  return 0;
}

// See the documentation in libc.h
int __wasilibc_open_nomode(const char *path, int oflag) {
  ic_trap("calling __wasilibc_open_nomode");
  return 0;
}

int access(const char *path, int amode) {
  ic_trap("calling access");
  return 0;
}

ssize_t readlink(
    const char *restrict path,
    char *restrict buf,
    size_t bufsize)
{
  ic_trap("calling readlink");
  return 0;
}

int stat(const char *restrict path, struct stat *restrict buf) {
  ic_trap("calling stat");
  return 0;
}

int lstat(const char *restrict path, struct stat *restrict buf) {
  ic_trap("calling lstat");
  return 0;
}

int utime(const char *path, const struct utimbuf *times) {
  ic_trap("calling utime");
  return 0;
}

int unlink(const char *path) {
  ic_trap("calling unlink");
  return 0;
}

int rmdir(const char *path) {
  ic_trap("calling rmdir");
  return 0;
}

int remove(const char *path) {
  ic_trap("calling remove");
  return 0;
}

int mkdir(const char *path, mode_t mode) {
  ic_trap("calling mkdir");
  return 0;
}

DIR *opendir(const char *dirname) {
  ic_trap("calling opendir");
  return 0;
}

int scandir(
    const char *restrict dir,
    struct dirent ***restrict namelist,
    int (*filter)(const struct dirent *),
    int (*compar)(const struct dirent **, const struct dirent **)
) {
  ic_trap("calling scandir");
  return 0;
}

int symlink(const char *target, const char *linkpath) {
  ic_trap("calling symlink");
  return 0;
}

int link(const char *old, const char *new) {
  ic_trap("calling link");
  return 0;
}

int rename(const char *old, const char *new) {
  ic_trap("calling rename");
  return 0;
}

int
__wasilibc_access(const char *path, int mode, int flags)
{
  ic_trap("calling __wasilibc_access");
  return 0;
}

int
__wasilibc_utimens(const char *path, const struct timespec times[2], int flags)
{
  ic_trap("calling __wasilibc_utimens");
  return 0;
}

int
__wasilibc_stat(const char *__restrict path, struct stat *__restrict st, int flags)
{
  ic_trap("calling __wasilibc_stat");
  return 0;
}

int
__wasilibc_link(const char *oldpath, const char *newpath, int flags)
{
  ic_trap("calling __wasilibc_link");
  return 0;
}

int
__wasilibc_link_oldat(int olddirfd, const char *oldpath, const char *newpath, int flags)
{
  ic_trap("calling __wasilibc_link_oldat");
  return 0;
}

int
__wasilibc_link_newat(const char *oldpath, int newdirfd, const char *newpath, int flags)
{
  ic_trap("calling __wasilibc_link_newat");
  return 0;
}

int
__wasilibc_rename_oldat(int fromdirfd, const char *from, const char *to)
{
  ic_trap("calling __wasilibc_rename_oldat");
  return 0;
}

int
__wasilibc_rename_newat(const char *from, int todirfd, const char *to)
{
  ic_trap("calling __wasilibc_rename_newat");
  return 0;
}
