// Exercises the 128-bit cycles & certified-data IC_APIs end-to-end.
// Each method returns '(0 : int)' on success and a non-zero code naming the
// failed check, so the same methods work as deployed pytest targets and as
// native MockIC run_test targets with a fixed expected hex.

#include "my_canister.h"

#include <vector>

#include "ic_api.h"

// '()' -> '(0 : int)'
// Callable both as a deployed ingress update (cycles available == 0) and
// from the native MockIC driver (leave the mock msg-cycles-available at 0).
void test_cycles() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  ic_api.from_wire();

  int result = 0;
  __uint128_t available = ic_api.get_msg_cycles_available();
  if (available != 0) result = 1; // an ingress message attaches no cycles
  if (ic_api.accept_msg_cycles(available) != available) result = 2;

  __uint128_t balance_before = ic_api.get_canister_self_cycle_balance();
  __uint128_t burned = ic_api.burn_cycles(1000000);
  if (burned != 1000000) result = 3;
  __uint128_t balance_after = ic_api.get_canister_self_cycle_balance();
  if (balance_before - balance_after != burned) result = 4;

  __uint128_t liquid = ic_api.get_canister_liquid_cycle_balance();
  if (liquid == 0 || liquid > balance_after) result = 5;

  ic_api.to_wire(CandidTypeInt{result});
}

// '()' -> '()'
// Certifies the fixed 32-byte value {0x00, 0x01, ..., 0x1f}. The native
// driver asserts afterwards that the mock stored exactly these bytes.
void test_certified_data_set() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  ic_api.from_wire();

  std::vector<uint8_t> data(32);
  for (size_t i = 0; i < data.size(); ++i) {
    data[i] = (uint8_t)i;
  }
  ic_api.set_certified_data(data);

  ic_api.to_wire();
}

// '()' -> '(0 : int)' when a data certificate is present & non-empty,
//          '(1 : int)' when absent (replicated context, or mock default),
//          '(2 : int)' when present but empty.
void test_data_certificate() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  ic_api.from_wire();

  int result = 0;
  if (!ic_api.data_certificate_present()) {
    result = 1;
  } else if (ic_api.get_data_certificate().empty()) {
    result = 2;
  }

  ic_api.to_wire(CandidTypeInt{result});
}
