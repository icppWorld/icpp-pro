#include "greet.h"

#include <string>

#include "ic_api.h"

// No heap memory allocations
void greet() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  ic_api.to_wire(CandidTypeText{"hello!"});
}

// Heap memory allocations that fits in 32-bit memory (<4Gb)
void greet32() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  ic_api.to_wire(CandidTypeText{"hello 32!"});
}

// Heap memory allocations that only fits in 64-bit memory (>4Gb)
void greet64() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  ic_api.to_wire(CandidTypeText{"hello 64!"});
}