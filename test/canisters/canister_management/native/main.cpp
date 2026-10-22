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

void check_eq(const std::string &actual, const std::string &expected,
              const std::string &what) {
  if (actual != expected) {
    std::cout << "Expecting:\n" << expected << "\nFound:\n" << actual << "\n";
  }
  check(actual == expected, what);
}

// Runs a method with a principal argument, and returns its text reply
std::string run_text(MockIC &mockIC, const std::string &name, void (*fn)(),
                     CandidArgs in, const std::string &caller) {
  std::string text;
  CandidArgs out;
  out.append(CandidTypeText{&text});
  std::string out_hex;
  mockIC.run_test(name, fn, hex(in), "", true, caller, &out_hex);
  decode(out_hex, out);
  return text;
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
  // ---------------------------------------------------------------------
  // The lifecycle helpers. The arguments are pinned as icpp encodes them;
  // each was checked with didc against the spec's ic.did (test/ic.did):
  //   didc decode -d test/ic.did -t '(<type>_args)' <hex>
  mockIC.clear_expected_calls();
  const std::string child{"ryjl3-tyaaa-aaaaa-aaaba-cai"};
  auto child_in = [&]() {
    CandidArgs in;
    in.append(CandidTypePrincipal{child});
    return in;
  };
  auto settings_in = [&](const std::string &name) {
    CandidArgs in;
    in.append(CandidTypePrincipal{child});
    in.append(CandidTypeText{name});
    return in;
  };
  {
    // update_settings, every field set; the Mock IC replies '()'
    check_eq(run_text(mockIC, "update_settings all", update_settings,
                      settings_in("all"), caller),
             "ok", "update_settings: all, the reply");
    check_eq(
        mockIC.last_call_arg_hex("aaaaa-aa", "update_settings"),
        "4449444c136c0dc0cff27103e8f09b73038ba08eaa0110d7e09b900202beb0ccfa020380ad988a0403edd9c8c9070698c8faa90903ad828e8c0a08f8e287cc0c03ffb9a9f60c0adeebb5a90e03a882acc60f036d686e016e7d6b01a981ceb7067f6b006e046b01caa989aa08016e076b01d7e09b90027f6e096c02f1fee18d030ccbe4fdc7040c6d716c006c02f1fee18d0371cbe4fdc704716d0e6e0f6c03b3c4b1f20468e3f9f5d90800ca9998b40d126e780111010a00000000000000020101010301080101013101410101010a0000000000c0115a01010106010401000107010001011d779590d2cd339802981dfd935d9a3dbb085cafe6ad19b87229a016d6020105010001020101010100000000000000",
        "update_settings: all, the argument");
    check(mockIC.last_call_cycles("aaaaa-aa", "update_settings") == 0,
          "update_settings: no cycles attached");
  }
  {
    // update_settings, no field set: the settings record is still sent
    check_eq(run_text(mockIC, "update_settings none", update_settings,
                      settings_in("none"), caller),
             "ok", "update_settings: none, the reply");
    check_eq(
        mockIC.last_call_arg_hex("aaaaa-aa", "update_settings"),
        "4449444c036c006c03b3c4b1f20468e3f9f5d90800ca9998b40d026e780101010a00000000000000020101010100000000000000",
        "update_settings: none, the argument");
  }
  {
    // update_settings, each visibility kind and the environment variables
    for (const char *name : {"public_logs", "viewers", "env", "limits"}) {
      check_eq(run_text(mockIC, std::string("update_settings ") + name,
                        update_settings, settings_in(name), caller),
               "ok", std::string("update_settings: ") + name);
    }
    check_eq(
        mockIC.last_call_arg_hex("aaaaa-aa", "update_settings"),
        "4449444c046c05c0cff27101e8f09b730180ad988a040198c8faa90901f8e287cc0c016e7d6c03b3c4b1f20468e3f9f5d90800ca9998b40d036e780102010a0000000000000002010101809a9e010180808080040180a0e5b9c2910101802001808080800c010100000000000000",
        "update_settings: limits, the argument");
  }
  {
    // update_settings, rejected
    mockIC.expect_call("aaaaa-aa", "update_settings",
                       MockIC::MockResponse::reject(
                           5, "Only the controllers of the canister can call"));
    check_eq(run_text(mockIC, "update_settings rejected", update_settings,
                      settings_in("none"), caller),
             "rej:5:Only the controllers of the canister can call",
             "update_settings: the reject");
    mockIC.clear_expected_calls();
  }
  {
    // start, stop, delete: the Mock IC replies '()'
    check_eq(run_text(mockIC, "start_child", start_child, child_in(), caller),
             "ok", "start_canister: the reply");
    check_eq(mockIC.last_call_arg_hex("aaaaa-aa", "start_canister"),
             "4449444c016c01b3c4b1f204680100010a00000000000000020101",
             "start_canister: the argument");
    check_eq(run_text(mockIC, "stop_child", stop_child, child_in(), caller),
             "ok", "stop_canister: the reply");
    check_eq(mockIC.last_call_arg_hex("aaaaa-aa", "stop_canister"),
             "4449444c016c01b3c4b1f204680100010a00000000000000020101",
             "stop_canister: the argument");
    check_eq(run_text(mockIC, "delete_child", delete_child, child_in(), caller),
             "ok", "delete_canister: the reply");
    check_eq(mockIC.last_call_arg_hex("aaaaa-aa", "delete_canister"),
             "4449444c016c01b3c4b1f204680100010a00000000000000020101",
             "delete_canister: the argument");
    check(mockIC.last_call_cycles("aaaaa-aa", "delete_canister") == 0,
          "delete_canister: no cycles attached");
  }
  {
    // delete_canister of a running canister: rejected; an expect_call
    // replaces the Mock IC's default reply
    mockIC.expect_call(
        "aaaaa-aa", "delete_canister",
        MockIC::MockResponse::reject(5, "Canister is not stopped"));
    check_eq(run_text(mockIC, "delete_child rejected", delete_child, child_in(),
                      caller),
             "rej:5:Canister is not stopped", "delete_canister: the reject");
    mockIC.clear_expected_calls();
  }
  {
    // stop_canister that times out: a bounded-wait call
    mockIC.expect_call("aaaaa-aa", "stop_canister",
                       MockIC::MockResponse::timeout());
    std::string text =
        run_text(mockIC, "stop_child timeout", stop_child, child_in(), caller);
    check(text.rfind("rej:6:", 0) == 0,
          "stop_canister: a timeout is SysUnknown");
    mockIC.clear_expected_calls();
  }
  {
    // deposit_cycles: the cycles are attached, and deposited
    CandidArgs in;
    in.append(CandidTypePrincipal{child});
    in.append(CandidTypeNat{__uint128_t(1'000'000)});
    check_eq(run_text(mockIC, "deposit_child", deposit_child, in, caller), "ok",
             "deposit_cycles: the reply");
    check_eq(mockIC.last_call_arg_hex("aaaaa-aa", "deposit_cycles"),
             "4449444c016c01b3c4b1f204680100010a00000000000000020101",
             "deposit_cycles: the argument");
    check(mockIC.last_call_cycles("aaaaa-aa", "deposit_cycles") == 1'000'000,
          "deposit_cycles: the cycles attached");
  }
  {
    // A method without a default reply and without expect_call: rejected
    check_eq(run_text(mockIC, "canister_status_full no response",
                      canister_status_full, child_in(), caller),
             "rej:3:Mock IC: no response for aaaaa-aa.canister_status - set "
             "one with expect_call",
             "canister_status: no response set");
  }
  {
    // The full canister_status, from a reply with every field of the spec,
    // the fields only the replica sends (controller, balance), and every
    // visibility kind. didc encode -t '(<the type of the replica's reply>)'
    // with module_hash = null; see the type in the PR of this test
    std::string reply =
        "4449444c0d6c0dc5df841c019cb1fa2568b2ceef2f029cbab69c0203ffdb81f7037dbc"
        "c1ee81067e98cec7e707788daacd94087de3f9f5d90806e8fc8cec090bb0e4d2970a7d"
        "81cfaef40a0c84aaa89e0f7d6c09a4a884027da2e6e2267dacdffc5d7da4b6f090067d"
        "bbfeb3af067d91bde3cf067da4cadab30d7de3abc9880e7da4cda1a70f7d6b038da487"
        "9b047ff496e4910b7fffdba5db0e7f6d046c020005017d6d7b6c0e9cb1fa2568c0cff2"
        "717de8f09b737d8ba08eaa0107d7e09b900209beb0ccfa027d80ad988a047dedd9c8c9"
        "070a98c8faa9097dad828e8c0a0af8e287cc0c7dffb9a9f60c0adeebb5a90e7da882ac"
        "c60f7d6d086c02f1fee18d0371cbe4fdc704716d686b03d7e09b90027fa981ceb7067f"
        "caa989aa08096c04c1f8dc83037d83cac6e9057da1d0b8af0a7d8fd0cfd00f7d6e0501"
        "000d12100f0b110a0c0e011d779590d2cd339802981dfd935d9a3dbb085cafe6ad19b8"
        "7229a016d6020101010163d20901090000000000000064011d779590d2cd339802981d"
        "fd935d9a3dbb085cafe6ad19b87229a016d602030802013101410374776f014201011d"
        "779590d2cd339802981dfd935d9a3dbb085cafe6ad19b87229a016d602050401060202"
        "011d779590d2cd339802981dfd935d9a3dbb085cafe6ad19b87229a016d602010a0000"
        "000000000002010107000201171514162a001e";
    mockIC.expect_call("aaaaa-aa", "canister_status",
                       MockIC::MockResponse::reply(reply));
    check_eq(
        run_text(mockIC, "canister_status_full", canister_status_full,
                 child_in(), caller),
        "status=stopping; ready_for_migration=true; version=9; controllers=1; "
        "compute_allocation=1; memory_allocation=2; freezing_threshold=3; "
        "reserved_cycles_limit=4; minimum_incoming_canister_call_cycles=5; "
        "log_visibility=public; log_memory_limit=6; "
        "snapshot_visibility=allowed_viewers(" +
            caller + "," + child +
            "); status_visibility=controllers; wasm_memory_limit=7; "
            "wasm_memory_threshold=8; environment_variables=A=1,B=two; "
            "module_hash=none; memory_size=1234; wasm_memory_size=10; "
            "log_memory_store_size=18; cycles=100; reserved_cycles=30; "
            "idle_cycles_burned_per_day=42; num_calls_total=20",
        "canister_status: every field");
    mockIC.clear_expected_calls();
  }
  {
    // The full canister_status from the minimal reply of an older replica:
    // the fields it leaves out stay at their defaults. didc encode -t
    // '(record { status : variant { running; stopping; stopped }; settings :
    // record { controllers : vec principal; freezing_threshold : nat };
    // module_hash : opt blob; memory_size : nat; cycles : nat;
    // idle_cycles_burned_per_day : nat })' with module_hash = null
    std::string reply =
        "4449444c066c06b2ceef2f01ffdb81f7037d8daacd94087de3f9f5d90802b0e4d2970a"
        "7d81cfaef40a046b038da4879b047ff496e4910b7fffdba5db0e7f6c02c0cff2717dd7"
        "e09b9002036d686e056d7b01000205060301011d779590d2cd339802981dfd935d9a3d"
        "bb085cafe6ad19b87229a016d6020700";
    mockIC.expect_call("aaaaa-aa", "canister_status",
                       MockIC::MockResponse::reply(reply));
    check_eq(
        run_text(mockIC, "canister_status_full minimal", canister_status_full,
                 child_in(), caller),
        "status=running; ready_for_migration=false; version=0; controllers=1; "
        "compute_allocation=0; memory_allocation=0; freezing_threshold=3; "
        "reserved_cycles_limit=0; minimum_incoming_canister_call_cycles=0; "
        "log_visibility=controllers; log_memory_limit=0; "
        "snapshot_visibility=controllers; status_visibility=controllers; "
        "wasm_memory_limit=0; wasm_memory_threshold=0; "
        "environment_variables=; module_hash=none; memory_size=5; "
        "wasm_memory_size=0; log_memory_store_size=0; cycles=6; "
        "reserved_cycles=0; idle_cycles_burned_per_day=7; num_calls_total=0",
        "canister_status: the minimal reply of an older replica");
    mockIC.clear_expected_calls();
  }
  {
    // canister_status with the wrong type on the wire: cycles as nat64 (the
    // minimal reply above, with cycles : nat64)
    std::string reply =
        "4449444c066c06b2ceef2f01ffdb81f7037d8daacd940878e3f9f5d90802b0e4d2970a"
        "7d81cfaef40a046b038da4879b047ff496e4910b7fffdba5db0e7f6c02c0cff2717dd7"
        "e09b9002036d686e056d7b0100020506000000000000000301011d779590d2cd339802"
        "981dfd935d9a3dbb085cafe6ad19b87229a016d6020700";
    mockIC.expect_call("aaaaa-aa", "canister_status",
                       MockIC::MockResponse::reply(reply));
    CandidArgs in = child_in();
    mockIC.run_trap_test("canister_status wrong type", canister_status_full,
                         hex(in), true, caller, "Wrong type on wire");
    mockIC.clear_expected_calls();
  }
  {
    // ECDSA with secp256r1: the key id, and the cost of the signature
    CandidArgs in;
    in.append(CandidTypeText{"test_key_1"});
    in.append(CandidTypeVecNat8{IC_Blob(32, 7)});
    check_eq(run_text(mockIC, "ecdsa_r1", ecdsa_r1, in, caller),
             "public_key=33 bytes; signature=64 bytes", "ecdsa secp256r1");
    check_eq(
        mockIC.last_call_arg_hex("aaaaa-aa", "ecdsa_public_key"),
        "4449444c066c03bbebadff0304b3c4b1f20401ada8b2b105036e686d7b6d026c02cbe4fdc70471af99e1f204056b01b3eae4ea017f01000a746573745f6b65795f310000010469637070",
        "ecdsa secp256r1: the key id");
    IC_API::SignCost cost = IC_API::cost_sign_with_ecdsa(
        "test_key_1", IC_API::EcdsaCurve::Secp256r1);
    check(cost.error == IC_API::SignCostError::None &&
              mockIC.last_call_cycles("aaaaa-aa", "sign_with_ecdsa") ==
                  cost.cycles,
          "ecdsa secp256r1: the cost attached");
    CandidArgs bad;
    bad.append(CandidTypeText{"no_such_key"});
    bad.append(CandidTypeVecNat8{IC_Blob(32, 7)});
    std::string text =
        run_text(mockIC, "ecdsa_r1 unknown key", ecdsa_r1, bad, caller);
    check(text.rfind("err:", 0) == 0, "ecdsa secp256r1: an unknown key name");
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
