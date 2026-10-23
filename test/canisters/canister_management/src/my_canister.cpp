// The management canister helpers (ic_management.h): each method makes one
// priced call for real, attaching the cycles of its cost_* function.

#include "my_canister.h"

#include <string>
#include <vector>

#include "ic_api.h"
#include "test_helpers.h"

namespace {

void reply_blobs(IC_API &ic_api, const IC_Blob &a, const IC_Blob &b) {
  CandidArgs out;
  out.append(CandidTypeVecNat8{a});
  out.append(CandidTypeVecNat8{b});
  ic_api.to_wire(out);
}

// The derivation path of the test keys
std::vector<IC_Blob> derivation_path() { return {{'i', 'c', 'p', 'p'}}; }
} // namespace

// '()' -> '(32 random bytes)'
IC_Async raw_rand_async() {
  IC_API ic_api(CanisterUpdate{"raw_rand"}, false);
  ic_api.from_wire();
  IC_CallResult<IC_Blob> r = co_await IC_Management::raw_rand();
  ic_api.to_wire(CandidTypeVecNat8{r.value()});
}
void raw_rand() { raw_rand_async(); }

// '()' -> '(principal of a new canister)', with 100B cycles to start with
IC_Async create_canister_async() {
  IC_API ic_api(CanisterUpdate{"create_canister"}, false);
  ic_api.from_wire();
  IC_CallResult<CandidTypePrincipal> r =
      co_await IC_Management::create_canister({}, 1'000'000'000'000);
  ic_api.to_wire(CandidTypePrincipal{r.value().get_text()});
}
void create_canister() { create_canister_async(); }

// '(canister_id)' -> '("running; cycles=..; module_hash=none; controllers=..; freezing_threshold=..")'
IC_Async canister_status_async() {
  IC_API ic_api(CanisterUpdate{"canister_status"}, false);
  std::string canister_id;
  ic_api.from_wire(CandidTypePrincipal{&canister_id});
  IC_CallResult<IC_CanisterStatus> r =
      co_await IC_Management::canister_status(CandidTypePrincipal{canister_id});
  if (!r) {
    ic_api.to_wire(CandidTypeText{error_text(r.error())});
    co_return;
  }
  const char *status = r->status == IC_API::CanisterStatus::Running ? "running"
                       : r->status == IC_API::CanisterStatus::Stopping
                           ? "stopping"
                           : "stopped";
  auto nat = [](__uint128_t v) {
    return std::to_string(static_cast<uint64_t>(v));
  };
  std::string module_hash =
      r->module_hash ? std::to_string(r->module_hash->size()) + " bytes"
                     : "none";
  ic_api.to_wire(CandidTypeText{
      std::string(status) + "; cycles=" + nat(r->cycles) +
      "; idle_cycles_burned_per_day=" + nat(r->idle_cycles_burned_per_day) +
      "; memory_size=" + nat(r->memory_size) + "; module_hash=" + module_hash +
      "; controllers=" + std::to_string(r->controllers.size()) +
      "; freezing_threshold=" + nat(r->freezing_threshold)});
}
void canister_status() { canister_status_async(); }

// '(key_name, message_hash)' -> '(public_key, signature)'
IC_Async ecdsa_async() {
  IC_API ic_api(CanisterUpdate{"ecdsa"}, false);
  std::string key_name;
  IC_Blob message_hash;
  CandidArgs args;
  args.append(CandidTypeText{&key_name});
  args.append(CandidTypeVecNat8{&message_hash});
  ic_api.from_wire(args);
  IC_EcdsaKeyId key{IC_API::EcdsaCurve::Secp256k1, key_name};

  IC_CallResult<IC_PublicKey> public_key =
      co_await IC_Management::ecdsa_public_key(key, derivation_path());
  IC_CallResult<IC_Blob> signature = co_await IC_Management::sign_with_ecdsa(
      message_hash, derivation_path(), key);
  reply_blobs(ic_api, public_key.value().public_key, signature.value());
}
void ecdsa() { ecdsa_async(); }

// '(key_name, ed25519, message)' -> '(public_key, signature)'
IC_Async schnorr_async() {
  IC_API ic_api(CanisterUpdate{"schnorr"}, false);
  std::string key_name;
  bool ed25519{true};
  IC_Blob message;
  CandidArgs args;
  args.append(CandidTypeText{&key_name});
  args.append(CandidTypeBool{&ed25519});
  args.append(CandidTypeVecNat8{&message});
  ic_api.from_wire(args);
  IC_SchnorrKeyId key{ed25519 ? IC_API::SchnorrAlgorithm::Ed25519
                              : IC_API::SchnorrAlgorithm::Bip340Secp256k1,
                      key_name};

  IC_CallResult<IC_PublicKey> public_key =
      co_await IC_Management::schnorr_public_key(key, derivation_path());
  IC_CallResult<IC_Blob> signature = co_await IC_Management::sign_with_schnorr(
      message, derivation_path(), key);
  reply_blobs(ic_api, public_key.value().public_key, signature.value());
}
void schnorr() { schnorr_async(); }

// '(key_name)' -> '(public_key, encrypted_key)'
IC_Async vetkd_async() {
  IC_API ic_api(CanisterUpdate{"vetkd"}, false);
  std::string key_name;
  ic_api.from_wire(CandidTypeText{&key_name});
  IC_VetKdKeyId key{IC_API::VetKdCurve::Bls12_381_G2, key_name};
  IC_Blob context{'i', 'c', 'p', 'p'};
  // The transport public key: a point of G1 (here its generator, compressed)
  IC_Blob transport_public_key{
      0x97, 0xf1, 0xd3, 0xa7, 0x31, 0x97, 0xd7, 0x94, 0x26, 0x95, 0x63, 0x8c,
      0x4f, 0xa9, 0xac, 0x0f, 0xc3, 0x68, 0x8c, 0x4f, 0x97, 0x74, 0xb9, 0x05,
      0xa1, 0x4e, 0x3a, 0x3f, 0x17, 0x1b, 0xac, 0x58, 0x6c, 0x55, 0xe8, 0x3f,
      0xf9, 0x7a, 0x1a, 0xef, 0xfb, 0x3a, 0xf0, 0x0a, 0xdb, 0x22, 0xc6, 0xbb};

  IC_CallResult<IC_Blob> public_key =
      co_await IC_Management::vetkd_public_key(key, context);
  IC_CallResult<IC_Blob> encrypted_key =
      co_await IC_Management::vetkd_derive_key(
          {'i', 'n', 'p', 'u', 't'}, context, transport_public_key, key);
  reply_blobs(ic_api, public_key.value(), encrypted_key.value());
}
void vetkd() { vetkd_async(); }

// A key that does not exist: the call is not made. '()' -> '("err:...")'
IC_Async invalid_key_async() {
  IC_API ic_api(CanisterUpdate{"invalid_key"}, false);
  ic_api.from_wire();
  IC_EcdsaKeyId key{IC_API::EcdsaCurve::Secp256k1, "no_such_key"};
  IC_CallResult<IC_Blob> r = co_await IC_Management::sign_with_ecdsa(
      IC_Blob(32, 0), derivation_path(), key);
  ic_api.to_wire(CandidTypeText{r ? "signed" : error_text(r.error())});
}
void invalid_key() { invalid_key_async(); }

// An HTTPS GET: replicated, through the transform below, or non-replicated.
// '(url, replicated)' -> '("<status> <body size> <number of headers>")'
IC_Async http_get_async() {
  IC_API ic_api(CanisterUpdate{"http_get"}, false);
  std::string url;
  bool replicated{true};
  CandidArgs args;
  args.append(CandidTypeText{&url});
  args.append(CandidTypeBool{&replicated});
  ic_api.from_wire(args);

  IC_HttpOutcallRequest request;
  request.url = url;
  request.max_response_bytes = 100'000;
  if (replicated) {
    request.transform_method = "transform";
    request.transform_context = {'n', 'o', ' ', 'h', 'e',
                                 'a', 'd', 'e', 'r', 's'};
  } else {
    request.is_replicated = false;
  }
  IC_CallResult<IC_HttpOutcallResponse> r =
      co_await IC_Management::http_request(request);
  ic_api.to_wire(CandidTypeText{r ? IC_API::to_string_128(r->status) + " " +
                                        std::to_string(r->body.size()) + " " +
                                        std::to_string(r->headers.size())
                                  : error_text(r.error())});
}
void http_get() { http_get_async(); }

// The transform of http_get: the nodes agree on the response once the
// headers, which differ per node (dates, ...), are removed.
void transform() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  IC_HttpTransformArgs args;
  ic_api.from_wire(args);
  args.response.headers.clear();
  ic_api.to_wire(args.response);
}

// The cost of a 1 kB request with a 10 kB response, pay-as-you-go:
// '()' -> '(fully replicated, non-replicated, flexible on 4 nodes)'
void cost_http_v2() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  ic_api.from_wire();
  IC_API::HttpOutcallCostParams params;
  params.request_bytes = 1'000;
  params.http_roundtrip_time_ms = 500;
  params.raw_response_bytes = 10'000;
  params.transformed_response_bytes = 10'000;
  params.transform_instructions = 1'000'000;
  __uint128_t fully = IC_API::cost_http_request_v2(params);
  params.outcall_type = IC_API::HttpOutcallType::NonReplicated;
  __uint128_t non_replicated = IC_API::cost_http_request_v2(params);
  params.outcall_type = IC_API::HttpOutcallType::Flexible;
  params.flexible_counts = IC_API::HttpOutcallReplicationCounts{4, 3, 4};
  __uint128_t flexible = IC_API::cost_http_request_v2(params);
  CandidArgs out;
  out.append(CandidTypeNat{fully});
  out.append(CandidTypeNat{non_replicated});
  out.append(CandidTypeNat{flexible});
  ic_api.to_wire(out);
}

// The Candid type vec blob, decoded and encoded: '(vec blob)' -> '(vec blob)'
void echo_vec_blob() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  std::vector<IC_Blob> blobs;
  ic_api.from_wire(CandidTypeVecBlob{&blobs});
  ic_api.to_wire(CandidTypeVecBlob{blobs});
}
