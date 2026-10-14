// Exercises the system-introspection, subnet, root-key & caller-info IC_APIs
// end-to-end. Each method returns '(0 : int)' on success and a non-zero code
// naming the failed check, so the same methods work as deployed pytest
// targets and as native MockIC run_test targets with a fixed expected hex.

#include "my_canister.h"

#include <vector>

#include "ic_api.h"

// '()' -> '(0 : int)'
void test_introspection() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  ic_api.from_wire();

  int result = 0;
  uint64_t counter_start = IC_API::performance_counter();
  if (ic_api.get_canister_version() < 1) result = 1;
  if (ic_api.get_canister_status() != IC_API::CanisterStatus::Running)
    result = 2;
  if (!IC_API::in_replicated_execution()) result = 3;
  if (ic_api.get_subnet_self().get_v_bytes().vec_uint8_t().empty()) result = 4;
  if (ic_api.get_subnet_node_count() == 0) result = 5;
  if (ic_api.get_root_key().size() != 133) result = 6;
  // An identity caller has no caller info
  if (!ic_api.get_caller_info_data().empty()) result = 7;
  if (ic_api.get_caller_info_signer().has_value()) result = 8;
  if (IC_API::performance_counter() <= counter_start) result = 9;
  if (IC_API::performance_counter(
          IC_API::PerformanceCounterType::CallContextInstructions) == 0)
    result = 10;

  ic_api.to_wire(CandidTypeInt{result});
}

// '()' -> '(0 : int)' when called as a query call (non-replicated)
void test_introspection_query() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  ic_api.from_wire();

  int result = 0;
  if (IC_API::in_replicated_execution()) result = 1;
  if (ic_api.get_canister_version() < 1) result = 2;
  if (ic_api.get_subnet_node_count() == 0) result = 3;
  if (!ic_api.get_caller_info_data().empty()) result = 4;

  ic_api.to_wire(CandidTypeInt{result});
}
