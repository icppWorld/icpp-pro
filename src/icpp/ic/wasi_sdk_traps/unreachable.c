// Modied from: https://github.com/ICPorts-labs/chico/blob/main/src/unreachable.c

#include <sys/socket.h>
#include <dirent.h>

#include <sys/uio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include "ic_trap.h"


#include <limits.h>
#include <errno.h>
#include <stdarg.h>

// All functions that are unsupported in Wasm

void *dlopen(const char *file, int mode) {
  ic_trap("calling dlopen");
  return 0;
}

char  *dlerror(void) {
  ic_trap("calling dlerror");
  return 0;
}

void *dlsym(void *restrict p, const char *restrict s){
  ic_trap("calling dlsym");
  return 0;
}

int dlclose(void *handle) {
  ic_trap("calling dlclose");
  return 0;
}

int __muloti4(int a, int b, int* overflow) {
  ic_trap("calling __muloti4");
  return 1;
}

long sysconf(int name) {
  ic_trap("calling sysconf");
  return 1;
}

int close(int fd) {
  ic_trap("calling close");
  return 1;
}

// is in posix.c
// int access(const char *pathname, int mode){
//   ic_trap("calling access");
//   return 1;
// }

int stat64(const char *pathname, int mode){
  ic_trap("calling stat64");
  return 1;
}

int fstat64(const char *pathname, int mode){
  ic_trap("calling fstat64");
  return 1;
}

int fchmod(const char *pathname, int mode){
  ic_trap("calling fchmod");
  return 1;
}

// is in posix.c
// int mkdir(const char *pathname, int mode){
//   ic_trap("calling mkdir");
//   return 1;
// }

int munmap(const char *pathname, int mode){
  ic_trap("calling munmap");
  return 1;
}

int lstat64(const char *pathname, int mode){
  ic_trap("calling lstat64");
  return 1;
}

int utimes(const char *pathname, int mode){
  ic_trap("calling utimes");
  return 1;
}

int gettimeofday(const char *pathname, int mode){
  ic_trap("calling gettimeofday");
  return 1;
}

int nanosleep(const char *pathname, int mode){
  ic_trap("calling nanosleep");
  return 1;
}

int __munmap(const char *pathname, int mode){
  ic_trap("calling __mumap");
  return 1;
}

int __map_file(const char *pathname, int mode){
  ic_trap("calling __map_file");
  return 1;
}

// is in posix.c
// int unlink(const char *pathname) {
//    ic_trap("calling unlink");
//    return 1;
// }

// is in posix.c
// int rmdir(const char *pathname) {
//    ic_trap("calling rmdir");
//    return 1;
// }

int fdatasync(int fd) {
   ic_trap("calling fdatasync");
   return 1;
}

int __expand_heap(const char *pathname) {
   ic_trap("calling __expand_heap");
   return 1;
}

ssize_t read(int fd, void *buf, size_t count){
   ic_trap("calling read");
   return 1;
}

void __lock(volatile int *l) {
   ic_trap("calling __lock");
}

void __unlock(volatile int *l) {
   ic_trap("calling __unlock");
}

int *___errno_location(void)
{
  ic_trap("calling __errno_location");
  return 0;
}

int *__errno_location(void)
{
  ic_trap("calling __errno_location");
  return 0;
}

uid_t geteuid(void)
{
  ic_trap("calling getetuid");
  return 1;
}

char *getcwd(char *buf, size_t size) {
   ic_trap("calling getcwd");
   return 0;
}

ssize_t write(int fd, const void *buf, size_t count)
{
  ic_trap("calling write");
  return 1;
}

ssize_t writev(int fildes, const struct iovec *iov, int iovcnt) 
{
  ic_trap("calling writev");
  return 1;
}

ssize_t fcntl64(int fd, const void *buf, size_t count)
{
  ic_trap("calling fcntl64");
  return 1;
}

int fchown(int fd, uid_t uid, gid_t gid)
{
  ic_trap("calling fchown");
  return 1;
}

// is in posix.c
// ssize_t readlink(const char *restrict path, char *restrict buf, size_t bufsize)
// {
//   ic_trap("calling readlink");
//   return 1;
// }

ssize_t open64(int fd, const void *buf, size_t count)
{
  ic_trap("calling open64");
  return 1;
}

pid_t getpid(void)
{
  ic_trap("calling getpid");
  return 1;
}

off_t __lseek(int fildes, off_t offset, int whence) {
  ic_trap("calling lseek");
  return 1;
}

off_t lseek64(int fd, off_t offset, int whence) {                                                                                                                               
  ic_trap("calling lseek64");
  return 1;
}

/*
This gives a warning during loading:

wasm-ld: warning: function signature mismatch: __multi3
>>> defined as (i32, i64, i64, i64, i64) -> void in unreachable.o
>>> defined as (i64, i64) -> i64 in lto.tmp
*/
//long long __multi3 (long long a , long long b ) {
  //ic_trap("calling __multi3");
  //return 1;
//}

long double __multf3 (long double a, long double b) {
  ic_trap("calling __multf3");
  return 1;
}

long double __subtf3 (long double a, long double b) {
  ic_trap("calling __subtf3");
  return 1;
}

long double __divtf3 (long double a, long double b) {
  ic_trap("calling __divtf3");
  return 1;
}

long double __addtf3 (long double a, long double b) {
  ic_trap("calling __addtf3");
  return 1;
}

void *mmap64(void *start, size_t len, int prot, int flags, int fd, off_t off) {
   ic_trap("calling mmap64");
  return 0;
}

int __letf2 (long double a, long double b) {
   ic_trap("calling __letf2");
   return 1;
}

int __getf2 (long double a, long double b) {
   ic_trap("calling __getf2");
   return 1;
}

int __lttf2 (long double a, long double b) {
   ic_trap("calling __lttf2");
   return 1;
}

int __gttf2 (long double a, long double b) {
   ic_trap("calling __gttf2");
   return 1;
}

void __floatditf (long a, long long b) {
  ic_trap("calling __floatditf");
}

void __floatsitf (int i, int j) {
   ic_trap("calling __floatsitf");
}

int __fixtfsi (long double a) {
   ic_trap("calling __fixtfsi");
   return 1;
}

double __trunctfdf2 (long double a) {
  ic_trap("calling __trunctfdf2");
  return 1;
}

long double __extenddftf2 (double a) {
  ic_trap("calling __extenddftf2 ");
  return 1;
}

int posix_fallocate64(int fd, off_t base, off_t len) {
  ic_trap("calling posix_fallocate64");
  return 1;
}

char *strerror(int e) {
  ic_trap("calling strerror");
  return 0;
}

off_t ftruncate64(int fd, off_t offset) {
  ic_trap("calling ftruncate64");
  return 1;
}

int vsnprintf(char *restrict s, size_t n, const char *restrict fmt, va_list ap) {
  ic_trap("calling vsnprintf");
  return 1;
}

int fcntl(int fildes, int cmd, ...) {
  ic_trap("calling fcntl");
  return 1;
}

int __wasilibc_nocwd_openat_nomode(int fd, const char *path, int oflag) {
  ic_trap("calling __wasilibc_nocwd_openat_nomode");
  return 1;
}

int ioctl(int fildes, int request, ...) {
  ic_trap("calling ioctl");
  return 1;
}

int getsockopt(int socket, int level, int option_name,
               void *restrict option_value, socklen_t *restrict option_len) {
  ic_trap("calling getsockopt");
  return 1;
}

int __wasilibc_nocwd_faccessat(int fd, const char *path, int amode, int flag) {
  ic_trap("calling __wasilibc_nocwd_faccessat");
  return 1;
}

ssize_t pread(int fildes, void *buf, size_t nbyte, off_t offset) {
  ic_trap("calling pread");
  return 1;
}

ssize_t pwrite(int fildes, const void *buf, size_t nbyte, off_t offset) {
  ic_trap("calling pwrite");
  return 1;
}

int __isatty(int fd) {
  ic_trap("calling __isatty");
  return 1;
}

DIR *fdopendir(int fd) {
  ic_trap("calling fdopendir");
  return 0;
}

struct dirent *readdir(DIR *dirp) {
  ic_trap("calling readdir");
  return 0;
}

int __wasilibc_nocwd_scandirat(int dirfd, const char *dir, struct dirent ***namelist,
                               int (*sel)(const struct dirent *),
                               int (*compar)(const struct dirent **, const struct dirent **)) {
  ic_trap("calling __wasilibc_nocwd_scandirat");
  return 1;
}

ssize_t readv(int fildes, const struct iovec *iov, int iovcnt) {
  ic_trap("calling readv");
  return 1;
}
