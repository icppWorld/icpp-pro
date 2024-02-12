#include "greet.h"

#include <string>

#include "ic_api.h"

// ================================================================
// Orthogonally Persisted std::vector

// Self managed pointer to a wrapped std::vector
class Counters {
public:
  std::vector<uint64_t> vec;
};

Counters *p_counters{nullptr};

// -----------------------------------------------------------------
void delete_memory() {
  if (p_counters) {
    delete p_counters;
    p_counters = nullptr;
  }
}

// Create a Counters instance with num_counters
void new_memory(uint64_t num_counters) {
  delete_memory();
  if (p_counters == nullptr) {

    IC_API::debug_print(std::string(__func__) +
                        ": Creating Counters Instance.");
    p_counters = new (std::nothrow) Counters();
    if (p_counters == nullptr) {
      IC_API::trap("Allocation of p_counters pointer failed");
    }

    IC_API::debug_print(std::string(__func__) + ": before call to reserve");
    p_counters->vec.reserve(num_counters);
    IC_API::debug_print(std::string(__func__) + ": after call to reserve");

    // Don't do this yet, you'll run into instruction limit per message before running out of memory...
    // p_counters->vec.resize(num_counters, 0); // Initialize counters to 0
  } else {
    IC_API::trap(std::string(__func__) + ": delete_memory failed");
  }
  IC_API::debug_print(std::string(__func__) +
                      "-----------------------------------------");
}

void add_counters_32(std::string called_from) {
  // Heap memory allocations that fits in 32-bit memory (<4Gb)
  for (size_t i = 0; i < 5; ++i) {
    uint64_t num_counters = i * 100000000;
    IC_API::debug_print(called_from +
                        ": num_counters = " + std::to_string(num_counters));
    new_memory(num_counters);
  }
}

void add_counters_64(std::string called_from) {
  // Heap memory allocations that fits in 32-bit memory (<4Gb)
  for (size_t i = 0; i < 10; ++i) {
    uint64_t num_counters = i * 100000000;
    IC_API::debug_print(called_from +
                        ": num_counters = " + std::to_string(num_counters));
    new_memory(num_counters);
  }
}

// -----------------------------------------------------------------

void query_greet() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  ic_api.to_wire(CandidTypeText{"hello!"});
}

void query_greet32() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  add_counters_32(std::string(__func__));
  ic_api.to_wire(CandidTypeText{"hello 32!"});
}

void query_greet64() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  add_counters_64(std::string(__func__));
  ic_api.to_wire(CandidTypeText{"hello 64!"});
}

// -----------------------------------------------------------------
void update_greet() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  ic_api.to_wire(CandidTypeText{"hello!"});
}

void update_greet32() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  add_counters_32(std::string(__func__));
  ic_api.to_wire(CandidTypeText{"hello 32!"});
}

void update_greet64() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  add_counters_64(std::string(__func__));
  ic_api.to_wire(CandidTypeText{"hello 64!"});
}