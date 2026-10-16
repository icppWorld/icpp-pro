// Typed helpers for the management canister (aaaaa-aa), as coroutines:
//
//   IC_Async my_method_async() {
//     IC_API ic_api(CanisterUpdate{"my_method"}, false);
//     IC_CallResult<std::vector<uint8_t>> r = co_await IC_Management::raw_rand();
//     ...
//   }
//
// Each helper attaches the cycles its call costs (cost_create_canister,
// cost_sign_with_ecdsa, ...). A call that attaches cycles is a
// guaranteed-response call, like in the Rust CDK: the management canister is
// trusted, and a bounded-wait call could lose the cycles on a timeout. The
// others are bounded-wait calls.
//
// Not available in C++17 builds, which have no coroutines.

#pragma once

#if defined(__cpp_impl_coroutine)

#include <optional>
#include <string>
#include <vector>

// docs start: ic_management
// The settings of a new canister; a field left empty gets the IC's default
struct IC_CanisterSettings {
  std::optional<std::vector<std::string>> controllers; // principal texts
  std::optional<__uint128_t> compute_allocation;
  std::optional<__uint128_t> memory_allocation;
  std::optional<__uint128_t> freezing_threshold;
  std::optional<__uint128_t> reserved_cycles_limit;
  std::optional<__uint128_t> wasm_memory_limit;
};

// The threshold keys, e.g. "key_1" on mainnet, "test_key_1" (mainnet and a
// local network) and "dfx_test_key" (a local network)
struct IC_EcdsaKeyId {
  IC_API::EcdsaCurve curve{IC_API::EcdsaCurve::Secp256k1};
  std::string name;
};
struct IC_SchnorrKeyId {
  IC_API::SchnorrAlgorithm algorithm{IC_API::SchnorrAlgorithm::Ed25519};
  std::string name;
};
struct IC_VetKdKeyId {
  IC_API::VetKdCurve curve{IC_API::VetKdCurve::Bls12_381_G2};
  std::string name;
};

struct IC_PublicKey {
  std::vector<uint8_t> public_key;
  std::vector<uint8_t> chain_code;
};

using IC_Blob = std::vector<uint8_t>;

class IC_Management {
public:
  // 32 random bytes
  static IC_Task<IC_CallResult<IC_Blob>> raw_rand();

  // Creates a canister, controlled by this canister unless settings say
  // otherwise. It starts with extra_cycles, attached on top of the cost of
  // the creation: a canister needs cycles to allocate memory, so give it some
  // (the IC rejects the creation with an "insufficient cycles" error
  // otherwise).
  static IC_Task<IC_CallResult<CandidTypePrincipal>>
  create_canister(IC_CanisterSettings settings = {},
                  __uint128_t extra_cycles = 0);

  // Threshold ECDSA
  static IC_Task<IC_CallResult<IC_PublicKey>>
  ecdsa_public_key(IC_EcdsaKeyId key, std::vector<IC_Blob> derivation_path,
                   std::optional<CandidTypePrincipal> canister_id = {});
  static IC_Task<IC_CallResult<IC_Blob>>
  sign_with_ecdsa(IC_Blob message_hash, std::vector<IC_Blob> derivation_path,
                  IC_EcdsaKeyId key);

  // Threshold Schnorr; bip341_merkle_root_hash only for bip340secp256k1
  static IC_Task<IC_CallResult<IC_PublicKey>>
  schnorr_public_key(IC_SchnorrKeyId key, std::vector<IC_Blob> derivation_path,
                     std::optional<CandidTypePrincipal> canister_id = {});
  static IC_Task<IC_CallResult<IC_Blob>>
  sign_with_schnorr(IC_Blob message, std::vector<IC_Blob> derivation_path,
                    IC_SchnorrKeyId key,
                    std::optional<IC_Blob> bip341_merkle_root_hash = {});

  // vetKD
  static IC_Task<IC_CallResult<IC_Blob>>
  vetkd_public_key(IC_VetKdKeyId key, IC_Blob context,
                   std::optional<CandidTypePrincipal> canister_id = {});
  static IC_Task<IC_CallResult<IC_Blob>>
  vetkd_derive_key(IC_Blob input, IC_Blob context, IC_Blob transport_public_key,
                   IC_VetKdKeyId key);

  // An HTTPS outcall
  static IC_Task<IC_CallResult<IC_HttpOutcallResponse>>
  http_request(IC_HttpOutcallRequest request);
}; // docs end: ic_management

#endif // __cpp_impl_coroutine
