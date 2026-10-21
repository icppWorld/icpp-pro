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

// The status of a canister, from IC_Management::canister_status. The reply
// has more fields; these are the ones decoded.
struct IC_CanisterStatus {
  IC_API::CanisterStatus status{IC_API::CanisterStatus::Running};
  __uint128_t cycles{0};
  __uint128_t idle_cycles_burned_per_day{0};
  __uint128_t memory_size{0};
  std::optional<IC_Blob> module_hash;   // empty: no code installed
  std::vector<std::string> controllers; // principal texts
  __uint128_t freezing_threshold{0};
};

// Calls to the management canister (aaaaa-aa), the IC's system API. Each
// helper encodes the Candid argument, attaches the cycles the call costs, and
// decodes the reply.
class IC_Management {
public:
  // 32 random bytes from the IC's random beacon, which no node can predict.
  // Execution is deterministic, so this is a canister's only source of secure
  // randomness.
  static IC_Task<IC_CallResult<IC_Blob>> raw_rand();

  // Creates an empty canister, e.g. one per user, controlled by this
  // canister unless settings say otherwise. It starts with extra_cycles, attached on top of the cost of
  // the creation: a canister needs cycles to allocate memory, so give it some
  // (the IC rejects the creation with an "insufficient cycles" error
  // otherwise).
  static IC_Task<IC_CallResult<CandidTypePrincipal>>
  create_canister(IC_CanisterSettings settings = {},
                  __uint128_t extra_cycles = 0);

  // The status of a canister this canister controls: running or stopped,
  // its cycles balance and burn rate, its memory, its module hash and its
  // controllers. The IC rejects the call if this canister is not a
  // controller.
  static IC_Task<IC_CallResult<IC_CanisterStatus>>
  canister_status(CandidTypePrincipal canister_id);

  // Threshold ECDSA (secp256k1): the private key is spread over the nodes of
  // a signing subnet and never exists in one place. The public key gives a
  // Bitcoin or Ethereum address that this canister controls; sign_with_ecdsa
  // signs a 32-byte hash, e.g. of a transaction.
  static IC_Task<IC_CallResult<IC_PublicKey>>
  ecdsa_public_key(IC_EcdsaKeyId key, std::vector<IC_Blob> derivation_path,
                   std::optional<CandidTypePrincipal> canister_id = {});
  static IC_Task<IC_CallResult<IC_Blob>>
  sign_with_ecdsa(IC_Blob message_hash, std::vector<IC_Blob> derivation_path,
                  IC_EcdsaKeyId key);

  // Threshold Schnorr: bip340secp256k1 for Bitcoin Taproot (with an optional
  // bip341_merkle_root_hash), ed25519 for e.g. Solana. Like ECDSA, no private
  // key exists in one place.
  static IC_Task<IC_CallResult<IC_PublicKey>>
  schnorr_public_key(IC_SchnorrKeyId key, std::vector<IC_Blob> derivation_path,
                     std::optional<CandidTypePrincipal> canister_id = {});
  static IC_Task<IC_CallResult<IC_Blob>>
  sign_with_schnorr(IC_Blob message, std::vector<IC_Blob> derivation_path,
                    IC_SchnorrKeyId key,
                    std::optional<IC_Blob> bip341_merkle_root_hash = {});

  // vetKD (vetKeys): derives a secret key for an input, encrypted to the
  // client's transport public key, so no node sees it. For end-to-end
  // encryption, with this canister deciding who gets which key.
  static IC_Task<IC_CallResult<IC_Blob>>
  vetkd_public_key(IC_VetKdKeyId key, IC_Blob context,
                   std::optional<CandidTypePrincipal> canister_id = {});
  static IC_Task<IC_CallResult<IC_Blob>>
  vetkd_derive_key(IC_Blob input, IC_Blob context, IC_Blob transport_public_key,
                   IC_VetKdKeyId key);

  // An HTTPS request to a server on the internet, e.g. a price feed
  static IC_Task<IC_CallResult<IC_HttpOutcallResponse>>
  http_request(IC_HttpOutcallRequest request);
}; // docs end: ic_management

#endif // __cpp_impl_coroutine
