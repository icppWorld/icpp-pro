// Exercises the cycle-cost IC_APIs. Returns '(0 : int)' on success and a
// non-zero code naming the failed check. The checks hold for any fee
// schedule, so the method runs on a local replica, on the IC and natively;
// the exact Mock IC values are asserted in native/main.cpp.

#include "my_canister.h"

#include <string>

#include "ic_api.h"

// '()' -> '(0 : int)'
void test_cost() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);

  using Error = IC_API::SignCostError;
  int result = 0;
  __uint128_t call = IC_API::cost_call(0, 0);
  if (call == 0) result = 1;
  if (IC_API::cost_call(10, 100) <= call) result = 2;
  if (IC_API::cost_create_canister() == 0) result = 3;
  if (IC_API::cost_http_request(100, 1000) <= IC_API::cost_http_request(0, 0))
    result = 4;

  IC_API::SignCost ecdsa =
      IC_API::cost_sign_with_ecdsa("key_1", IC_API::EcdsaCurve::Secp256k1);
  if (ecdsa.error != Error::None || ecdsa.cycles == 0) result = 5;
  IC_API::SignCost ed25519 = IC_API::cost_sign_with_schnorr(
      "test_key_1", IC_API::SchnorrAlgorithm::Ed25519);
  if (ed25519.error != Error::None || ed25519.cycles == 0) result = 6;
  IC_API::SignCost vetkd = IC_API::cost_vetkd_derive_key(
      "test_key_1", IC_API::VetKdCurve::Bls12_381_G2);
  if (vetkd.error != Error::None || vetkd.cycles == 0) result = 7;

  // Algorithm 2 does not exist
  IC_API::SignCost bad_algorithm = IC_API::cost_sign_with_schnorr(
      "key_1", static_cast<IC_API::SchnorrAlgorithm>(2));
  if (bad_algorithm.error != Error::InvalidCurveOrAlgorithm ||
      bad_algorithm.cycles != 0)
    result = 8;
  IC_API::SignCost bad_key = IC_API::cost_sign_with_ecdsa(
      "no_such_key", IC_API::EcdsaCurve::Secp256k1);
  if (bad_key.error != Error::InvalidKeyName || bad_key.cycles != 0) result = 9;

  ic_api.to_wire(CandidTypeInt{result});
}
