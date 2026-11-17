// Exercises IC_API::get_stable_size, which reads the 64-bit ic0_stable64_size.
// On the IC the file system keeps its files in stable memory, so the size is
// above 0 there. Natively it is what the test sets with
// mockIC.set_stable_size.

#include "my_canister.h"

#include <string>

#include "ic0.h"
#include "ic_api.h"

// '()' -> '(pages : nat64)'. Traps when the 32-bit ic0_stable_size disagrees.
void test_stable_size() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  ic_api.from_wire();

  uint64_t pages = IC_API::get_stable_size();
  uint32_t pages_32 = ic0_stable_size();
  if (pages != pages_32) {
    IC_API::trap(std::string(__func__) + ": ic0_stable64_size gives " +
                 std::to_string(pages) + " pages, ic0_stable_size " +
                 std::to_string(pages_32));
  }

  ic_api.to_wire(CandidTypeNat64{pages});
}
