// Main entry point for a native debug executable.
// Build it with: `icpp build-native` from the parent folder where 'icpp.toml'
// resides.
//
// The Mock IC answers the management canister (aaaaa-aa) itself, with
// deterministic bytes of the right length; an HTTPS outcall gets the response
// set with expect_call. The deployed tests make the calls for real.

#include "main.h"

#include <iostream>
#include <string>

#include "../src/my_canister.h"

// The Mock IC
// clang-format off
#include "ic_api.h"
#include "icpp_hooks.h"
#include "mock_ic.h"
#include "ic0.h"
// clang-format on

namespace {
void check(bool ok, const std::string &what) {
  if (!ok) {
    std::cout << "Test '" << what << "' Failed\n";
    exit(1);
  }
  std::cout << "Test '" << what << "' - Passed\n";
}

std::string hex(CandidArgs &args) {
  VecBytes bytes = CandidSerialize(args).get_B();
  return bytes.as_hex_string();
}

void decode(const std::string &hex, CandidArgs &args) {
  VecBytes bytes;
  bytes.store_hex_string(hex);
  CandidDeserialize(bytes, args);
}

// Runs a method, and decodes its reply into args
void run(MockIC &mockIC, const std::string &name, void (*fn)(), CandidArgs in,
         CandidArgs &out, const std::string &caller) {
  std::string out_hex;
  mockIC.run_test(name, fn, hex(in), "", true, caller, &out_hex);
  decode(out_hex, out);
}
} // namespace

int main() {
  bool exit_on_fail = true;
  MockIC mockIC(exit_on_fail);
  std::string caller{
      "expmt-gtxsw-inftj-ttabj-qhp5s-nozup-n3bbo-k7zvn-dg4he-knac3-lae"};
  mockIC.register_method("transform", transform, MockIC::MethodKind::Query);

  {
    IC_Blob a, b;
    CandidArgs out_a, out_b;
    out_a.append(CandidTypeVecNat8{&a});
    out_b.append(CandidTypeVecNat8{&b});
    run(mockIC, "raw_rand", raw_rand, CandidArgs{}, out_a, caller);
    run(mockIC, "raw_rand again", raw_rand, CandidArgs{}, out_b, caller);
    check(a.size() == 32 && b.size() == 32 && a != b,
          "raw_rand: 32 bytes, different each call");
  }
  {
    std::string id;
    CandidArgs out;
    out.append(CandidTypePrincipal{&id});
    run(mockIC, "create_canister", create_canister, CandidArgs{}, out, caller);
    check(!id.empty(), "create_canister: a new principal");
  }
  {
    CandidArgs in;
    in.append(CandidTypeText{"test_key_1"});
    in.append(CandidTypeVecNat8{IC_Blob(32, 7)});
    IC_Blob public_key, signature;
    CandidArgs out;
    out.append(CandidTypeVecNat8{&public_key});
    out.append(CandidTypeVecNat8{&signature});
    run(mockIC, "ecdsa", ecdsa, in, out, caller);
    check(public_key.size() == 33 && signature.size() == 64,
          "ecdsa: a public key and a signature");
  }
  {
    CandidArgs in;
    in.append(CandidTypeText{"test_key_1"});
    in.append(CandidTypeBool{true});
    in.append(CandidTypeVecNat8{IC_Blob{'h', 'i'}});
    IC_Blob public_key, signature;
    CandidArgs out;
    out.append(CandidTypeVecNat8{&public_key});
    out.append(CandidTypeVecNat8{&signature});
    run(mockIC, "schnorr", schnorr, in, out, caller);
    check(signature.size() == 64, "schnorr: a signature");
  }
  {
    CandidArgs in;
    in.append(CandidTypeText{"test_key_1"});
    IC_Blob public_key, encrypted_key;
    CandidArgs out;
    out.append(CandidTypeVecNat8{&public_key});
    out.append(CandidTypeVecNat8{&encrypted_key});
    run(mockIC, "vetkd", vetkd, in, out, caller);
    check(public_key.size() == 96 && encrypted_key.size() == 192,
          "vetkd: a public key and an encrypted key");
  }
  {
    std::string text;
    CandidArgs out;
    out.append(CandidTypeText{&text});
    run(mockIC, "invalid_key", invalid_key, CandidArgs{}, out, caller);
    check(text == "err:unknown key name", "invalid_key: not signed");
  }
  {
    // canister_status: the reply has more fields than IC_CanisterStatus,
    // as the real one does; they are skipped
    CandidTypeRecord settings;
    settings.append("controllers",
                    CandidTypeVecPrincipal{std::vector<std::string>{caller}});
    settings.append("compute_allocation", CandidTypeNat{__uint128_t(0)});
    settings.append("memory_allocation", CandidTypeNat{__uint128_t(0)});
    settings.append("freezing_threshold", CandidTypeNat{__uint128_t(2592000)});
    settings.append("reserved_cycles_limit",
                    CandidTypeNat{__uint128_t(5000000000000)});
    settings.append("log_visibility", CandidTypeVariant{"controllers"});
    settings.append("wasm_memory_limit", CandidTypeNat{__uint128_t(0)});
    CandidTypeRecord memory_metrics;
    memory_metrics.append("wasm_memory_size", CandidTypeNat{__uint128_t(100)});
    memory_metrics.append("stable_memory_size", CandidTypeNat{__uint128_t(0)});
    CandidTypeRecord query_stats;
    query_stats.append("num_calls_total", CandidTypeNat{__uint128_t(0)});
    CandidTypeRecord result;
    result.append("status", CandidTypeVariant{"stopped"});
    result.append("settings", settings);
    result.append("module_hash",
                  CandidTypeOptVec{CandidTypeVecNat8{IC_Blob(32, 0xab)}});
    result.append("memory_size", CandidTypeNat{__uint128_t(1234)});
    result.append("memory_metrics", memory_metrics);
    result.append("cycles", CandidTypeNat{__uint128_t(100000000000)});
    result.append("reserved_cycles", CandidTypeNat{__uint128_t(0)});
    result.append("idle_cycles_burned_per_day", CandidTypeNat{__uint128_t(42)});
    result.append("query_stats", query_stats);
    CandidArgs reply;
    reply.append(result);
    mockIC.expect_call("aaaaa-aa", "canister_status",
                       MockIC::MockResponse::reply(hex(reply)));
    CandidArgs in;
    in.append(CandidTypePrincipal{"aaaaa-aa"});
    std::string text;
    CandidArgs out;
    out.append(CandidTypeText{&text});
    run(mockIC, "canister_status", canister_status, in, out, caller);
    check(text ==
              "stopped; cycles=100000000000; idle_cycles_burned_per_day=42; "
              "memory_size=1234; module_hash=32 bytes; controllers=1; "
              "freezing_threshold=2592000",
          "canister_status: the decoded fields");
  }
  {
    // canister_status of a canister this canister does not control
    mockIC.expect_call(
        "aaaaa-aa", "canister_status",
        MockIC::MockResponse::reject(5, "Only controllers can call it"));
    CandidArgs in;
    in.append(CandidTypePrincipal{"aaaaa-aa"});
    std::string text;
    CandidArgs out;
    out.append(CandidTypeText{&text});
    run(mockIC, "canister_status (rejected)", canister_status, in, out, caller);
    check(text == "rej:5:Only controllers can call it",
          "canister_status: the reject");
  }
  {
    // An HTTPS outcall: the response set with expect_call
    CandidTypeRecord header;
    header.append("name", CandidTypeVecText{std::vector<std::string>{"a"}});
    header.append("value", CandidTypeVecText{std::vector<std::string>{"b"}});
    CandidTypeRecord result;
    result.append("status", CandidTypeNat{__uint128_t(200)});
    result.append("headers", CandidTypeVecRecord{header});
    result.append("body", CandidTypeVecNat8{IC_Blob{'h', 'i'}});
    CandidArgs reply;
    reply.append(result);
    mockIC.expect_call("aaaaa-aa", "http_request",
                       MockIC::MockResponse::reply(hex(reply)));
    CandidArgs in;
    in.append(CandidTypeText{"https://example.com"});
    in.append(CandidTypeBool{false});
    std::string text;
    CandidArgs out;
    out.append(CandidTypeText{&text});
    run(mockIC, "http_get", http_get, in, out, caller);
    check(text == "200 2 1", "http_get: status, body size, headers");
  }
  {
    // The transform removes the headers
    CandidTypeRecord header;
    header.append("name", CandidTypeVecText{std::vector<std::string>{"date"}});
    header.append("value", CandidTypeVecText{std::vector<std::string>{"now"}});
    CandidTypeRecord response;
    response.append("status", CandidTypeNat{__uint128_t(200)});
    response.append("headers", CandidTypeVecRecord{header});
    response.append("body", CandidTypeVecNat8{IC_Blob{'h', 'i'}});
    CandidTypeRecord arg;
    arg.append("response", response);
    arg.append("context", CandidTypeVecNat8{IC_Blob{}});
    CandidArgs in;
    in.append(arg);
    __uint128_t status{0};
    std::vector<std::string> names, values;
    IC_Blob body;
    CandidTypeRecord r_header;
    r_header.append("name", CandidTypeVecText{&names});
    r_header.append("value", CandidTypeVecText{&values});
    CandidTypeRecord r_result;
    r_result.append("status", CandidTypeNat{&status});
    r_result.append("headers", CandidTypeVecRecord{&r_header});
    r_result.append("body", CandidTypeVecNat8{&body});
    CandidArgs out;
    out.append(r_result);
    run(mockIC, "transform", transform, in, out, caller);
    check(status == 200 && names.empty() && body.size() == 2,
          "transform: the headers removed");
  }
  {
    __uint128_t fully{0}, non_replicated{0}, flexible{0};
    CandidArgs out;
    out.append(CandidTypeNat{&fully});
    out.append(CandidTypeNat{&non_replicated});
    out.append(CandidTypeNat{&flexible});
    run(mockIC, "cost_http_v2", cost_http_v2, CandidArgs{}, out, caller);
    check(fully > flexible && flexible > non_replicated && non_replicated > 0,
          "cost_http_v2: fully > flexible on 4 nodes > non-replicated");
  }

  {
    std::vector<IC_Blob> in_blobs{{1, 2}, {}, {'a'}}, out_blobs;
    CandidArgs in;
    in.append(CandidTypeVecBlob{in_blobs});
    CandidArgs out;
    out.append(CandidTypeVecBlob{&out_blobs});
    run(mockIC, "echo_vec_blob", echo_vec_blob, in, out, caller);
    check(out_blobs == in_blobs, "echo_vec_blob: the same blobs back");
  }

  // returns 1 if any tests failed
  return mockIC.test_summary();
}
