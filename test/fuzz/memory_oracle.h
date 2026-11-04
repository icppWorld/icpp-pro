// The memory a fuzz input makes the decoder hold, at its peak.
//
// A message of a few bytes must not make the decoder hold much more memory
// than its size: that is how a canister runs out of memory on a small
// message. libFuzzer's own limits (-malloc_limit_mb, -rss_limit_mb) see one
// allocation, or the whole process; this sees what one input allocates at
// most at a time, through ASan's malloc and free hooks, and aborts when that
// is over MemoryOracle::limit(size), which leaves the input in
// build-fuzz/crashes/.

#pragma once

#include <sanitizer/allocator_interface.h>

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <new>

class MemoryOracle {
public:
  // Bytes per byte of the message, and a fixed allowance: a decoded value
  // takes a few hundred bytes, and a message byte can be a whole value
  static constexpr size_t PER_BYTE = 2048;
  static constexpr size_t FIXED = size_t(1) << 20;

  static size_t limit(size_t size) { return PER_BYTE * size + FIXED; }

  // Starts measuring from what is allocated now
  static void start() {
    install();
    s_start = s_live;
    s_peak = s_live;
  }

  // Aborts when the peak since start() is over the limit for the size
  static void check(size_t size, const char *what) {
    size_t used = s_peak - s_start;
    if (used <= limit(size)) return;
    std::fprintf(stderr,
                 "==MemoryOracle== %s: %zu bytes held at the peak, for a "
                 "message of %zu bytes; the limit is %zu\n",
                 what, used, size, limit(size));
    std::abort();
  }

private:
  static inline size_t s_live = 0;
  static inline size_t s_peak = 0;
  static inline size_t s_start = 0;

  static void on_malloc(const volatile void *, size_t size) {
    s_live += size;
    s_peak = std::max(s_peak, s_live);
  }
  static void on_free(const volatile void *p) {
    // The hook runs before the memory is freed, so its size is still known
    s_live -= __sanitizer_get_allocated_size(const_cast<const void *>(p));
  }

  // Once: install the hooks, and check that they count what is allocated
  static void install() {
    static bool done = [] {
      __sanitizer_install_malloc_and_free_hooks(on_malloc, on_free);
      // ::operator new itself, which the compiler may not leave out, as it
      // may a new-expression whose memory is not used
      size_t before = s_live;
      void *volatile p = ::operator new(4096);
      bool counted = s_live - before == 4096;
      ::operator delete(p);
      if (!counted || s_live != before) {
        std::fprintf(stderr, "==MemoryOracle== the malloc hooks do not "
                             "count allocations\n");
        std::abort();
      }
      return true;
    }();
    (void)done;
  }
};
