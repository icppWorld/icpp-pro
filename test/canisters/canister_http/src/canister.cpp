// Initialization of the canister
#include "canister.h"

#include <algorithm>
#include <memory>
#include <string>
#include <variant>

#include "ic_api.h"

// Orthogonally Persisted counter

// This results in an error after an upate call
// Replica Error: reject code CanisterError, reject message IC0502: Canister bkyz2-fmaaa-aaaaa-qaaaq-cai trapped: heap out of bounds, error code Some("IC0502")
//
// uint64_t counter{0};

Counter *p_counter{nullptr};

void canister_init() {
  IC_API ic_api(CanisterInit{std::string(__func__)}, false);

  // Create a Counter instance
  if (p_counter == nullptr) {
    IC_API::debug_print(std::string(__func__) + ": Creating Counter Instance.");
    p_counter = new (std::nothrow) Counter();
    if (p_counter == nullptr) {
      IC_API::trap("Allocation of p_counter failed");
    }
  }
}