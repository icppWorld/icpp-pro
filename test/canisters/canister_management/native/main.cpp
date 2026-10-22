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
#include "../src/test_helpers.h"

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
  // ---------------------------------------------------------------------
  // The code helpers. The arguments are pinned as icpp encodes them, each
  // checked with didc against test/ic.did; the replies are didc encode-d.
  {
    // install_code in every mode; the Mock IC replies '()'
    const char *modes[] = {"install", "reinstall", "upgrade", "upgrade_skip",
                           "upgrade_keep"};
    const char *args[] = {
        "4449444c046c05d6fca70202a79fc97e02e3a683c30401b3c4b1f20468ca9998b40d036b019baaebec087f6d7b6e780100024449040061736d00010a00000000000000020101010100000000000000",
        "4449444c046c05d6fca70202a79fc97e02e3a683c30401b3c4b1f20468ca9998b40d036b01c8bb8a707f6d7b6e780100024449040061736d00010a00000000000000020101010100000000000000",
        "4449444c066c006e006b019ce9c69906016c05d6fca70204a79fc97e04e3a683c30402b3c4b1f20468ca9998b40d056d7b6e780103024449040061736d0001010a00000000000000020101010100000000000000",
        "4449444c086c006e7e6c01c0a09bde05016e026b019ce9c69906036c05d6fca70206a79fc97e06e3a683c30404b3c4b1f20468ca9998b40d076d7b6e780105024449040061736d00010101010a00000000000000020101010100000000000000",
        "4449444c0a6c006b0185a19bb8047f6b006e016c01dceaaefa02036e046b019ce9c69906056c05d6fca70208a79fc97e08e3a683c30406b3c4b1f20468ca9998b40d096d7b6e780107024449040061736d00010100010a00000000000000020101010100000000000000"};
    for (int i = 0; i < 5; ++i) {
      CandidArgs in;
      in.append(CandidTypePrincipal{child});
      in.append(CandidTypeText{modes[i]});
      in.append(CandidTypeVecNat8{IC_Blob{0x00, 0x61, 0x73, 0x6d}});
      in.append(CandidTypeVecNat8{IC_Blob{0x44, 0x49}});
      check_eq(run_text(mockIC, std::string("install_child ") + modes[i],
                        install_child, in, caller),
               "ok", std::string("install_code: ") + modes[i]);
      check_eq(mockIC.last_call_arg_hex("aaaaa-aa", "install_code"), args[i],
               std::string("install_code: the argument, ") + modes[i]);
    }
  }
  {
    // upload_chunk: the reply is the chunk's hash
    mockIC.expect_call(
        "aaaaa-aa", "upload_chunk",
        MockIC::MockResponse::reply(
            "4449444c026c01cedfa0a804016d7b010020ababababababababababababababababab"
            "ababababababababababababababab"));
    CandidArgs in;
    in.append(CandidTypePrincipal{child});
    in.append(CandidTypeVecNat8{IC_Blob(5, 0x11)});
    check_eq(
        run_text(mockIC, "upload_chunk_child", upload_chunk_child, in, caller),
        hex_text(IC_Blob(32, 0xab)), "upload_chunk: the hash");
    check_eq(
        mockIC.last_call_arg_hex("aaaaa-aa", "upload_chunk"),
        "4449444c026c02ed8c8bae0401b3c4b1f204686d7b0100051111111111010a00000000000000020101",
        "upload_chunk: the argument");
    mockIC.clear_expected_calls();
  }
  {
    // stored_chunks: two hashes, then none
    mockIC.expect_call(
        "aaaaa-aa", "stored_chunks",
        MockIC::MockResponse::reply(
            "4449444c036d016c01cedfa0a804026d7b010002020102020304"));
    check_eq(run_text(mockIC, "stored_chunks_child", stored_chunks_child,
                      child_in(), caller),
             "0102,0304", "stored_chunks: two hashes");
    mockIC.expect_call("aaaaa-aa", "stored_chunks",
                       MockIC::MockResponse::reply(
                           "4449444c036d016c01cedfa0a804026d7b010000"));
    check_eq(run_text(mockIC, "stored_chunks_child empty", stored_chunks_child,
                      child_in(), caller),
             "", "stored_chunks: none");
    check_eq(mockIC.last_call_arg_hex("aaaaa-aa", "stored_chunks"),
             "4449444c016c01b3c4b1f204680100010a00000000000000020101",
             "stored_chunks: the argument");
    mockIC.clear_expected_calls();
  }
  {
    // clear_chunk_store and uninstall_code: the Mock IC replies '()'
    check_eq(run_text(mockIC, "clear_chunks_child", clear_chunks_child,
                      child_in(), caller),
             "ok", "clear_chunk_store: the reply");
    check_eq(mockIC.last_call_arg_hex("aaaaa-aa", "clear_chunk_store"),
             "4449444c016c01b3c4b1f204680100010a00000000000000020101",
             "clear_chunk_store: the argument");
    check_eq(run_text(mockIC, "uninstall_child", uninstall_child, child_in(),
                      caller),
             "ok", "uninstall_code: the reply");
    check_eq(
        mockIC.last_call_arg_hex("aaaaa-aa", "uninstall_code"),
        "4449444c026c02b3c4b1f20468ca9998b40d016e780100010a00000000000000020101010100000000000000",
        "uninstall_code: the argument");
  }
  {
    // install_chunked_code, from the target's own store and from another
    for (int with_store = 0; with_store < 2; ++with_store) {
      CandidArgs in;
      in.append(CandidTypePrincipal{child});
      in.append(CandidTypeText{"upgrade"});
      in.append(CandidTypeVecBlob{std::vector<IC_Blob>{{1, 2}, {3, 4}}});
      in.append(CandidTypeVecNat8{IC_Blob{9, 9}});
      in.append(CandidTypeOptPrincipal{
          with_store ? std::optional<std::string>{"aaaaa-aa"} : std::nullopt});
      check_eq(run_text(mockIC, "install_chunked_child", install_chunked_child,
                        in, caller),
               "ok", "install_chunked_code: the reply");
      check_eq(
          mockIC.last_call_arg_hex("aaaaa-aa", "install_chunked_code"),
          with_store
              ? "4449444c0b6c01cedfa0a804026d7b6d016c006e036b019ce9c69906046c07d6fca70201a6bd87b10401e3a683c30405eff5f3f0060995e8dcba0a688583b6af0b07ca9998b40d0a6e686c01cedfa0a804016d086e78010600020909000102020102020304010a00000000000000020101010100010100000000000000"
              : "4449444c0b6c01cedfa0a804026d7b6d016c006e036b019ce9c69906046c07d6fca70201a6bd87b10401e3a683c30405eff5f3f0060995e8dcba0a688583b6af0b07ca9998b40d0a6e686c01cedfa0a804016d086e78010600020909000102020102020304010a0000000000000002010100010100000000000000",
          "install_chunked_code: the argument");
    }
  }
  {
    // install_chunked_code with a wrong hash: rejected
    mockIC.expect_call(
        "aaaaa-aa", "install_chunked_code",
        MockIC::MockResponse::reject(5, "Wasm module hash does not match"));
    CandidArgs in;
    in.append(CandidTypePrincipal{child});
    in.append(CandidTypeText{"install"});
    in.append(CandidTypeVecBlob{std::vector<IC_Blob>{{1, 2}}});
    in.append(CandidTypeVecNat8{IC_Blob{0}});
    in.append(CandidTypeOptPrincipal{std::optional<std::string>{}});
    check_eq(run_text(mockIC, "install_chunked_child rejected",
                      install_chunked_child, in, caller),
             "rej:5:Wasm module hash does not match",
             "install_chunked_code: the reject");
    mockIC.clear_expected_calls();
  }
  {
    // canister_metadata
    mockIC.expect_call(
        "aaaaa-aa", "canister_metadata",
        MockIC::MockResponse::reply(
            "4449444c026c01f1fee18d03016d7b01000b7075626c69632d64617461"));
    CandidArgs in;
    in.append(CandidTypePrincipal{child});
    in.append(CandidTypeText{"icpp"});
    check_eq(run_text(mockIC, "metadata_child", metadata_child, in, caller),
             "value=public-data", "canister_metadata: the value");
    check_eq(
        mockIC.last_call_arg_hex("aaaaa-aa", "canister_metadata"),
        "4449444c016c02cbe4fdc70471b3c4b1f2046801000469637070010a00000000000000020101",
        "canister_metadata: the argument");
    mockIC.clear_expected_calls();
  }
  {
    // canister_info: every kind of change, both origins
    mockIC.expect_call(
        "aaaaa-aa", "canister_info",
        MockIC::MockResponse::reply(
            "4449444c166c04d7e09b90020181cfaef40a029ff4c1b60b048fedd8b10e786d686e03"
            "6d7b6d056c04d6f68e800178c0c3dff50278e6b384d80406c2b9dbda0a0a6b0280d1e8"
            "900207dced83b40b086c018fc1d4fb06686c02c0c3dff50209b3c4b1f204686e786b07"
            "9f90dedf020bd798fbac040cbd81f4ce040e8cb1bb9a0511c89cb4c90a1398f795de0a"
            "1590c6909a0f7f6c02d7e09b900201c2e7e2ce0b026c02e3a683c3040d81cfaef40a03"
            "6b03c8bb8a707f9ce9c699067f9baaebec087f6c05c0c3dff50278dbe1c691030f9ed9"
            "c0ad081082bff3a50d78b6b897890f036b0291a0c58d0370a4f794e508706e686c02d7"
            "e09b900212c2e7e2ce0b026e016c04dca5dce70214b3c4b1f20468c8b6fa9307688fed"
            "d8b10e786c03b3c4b1f2046898cec7e707788fedd8b10e786c01d7e09b900201010002"
            "01000101040102abcd0701000000000000000000000000000000000101040001010001"
            "02010202000000000000000100000000000000010105000000000000000100010104ab"
            "cdef010300000000000000020000000000000001000100060400000000000000030000"
            "0000000000000100020200000000000000000009000000000000000200010500000000"
            "0000000400000000000000000100050006000000000000000500000000000000000100"
            "0301010101040007000000000000000600000000000000000100040101040100000000"
            "00000004000000000000000100010003000000000000000c00000000000000"));
    CandidArgs in;
    in.append(CandidTypePrincipal{child});
    in.append(CandidTypeOptNat64{std::optional<uint64_t>{20}});
    check_eq(
        run_text(mockIC, "info_child", info_child, in, caller),
        "total=12; controllers=2; module_hash=abcd; changes=creation(1,env)/user "
        "code_deployment(upgrade,abcdef01)/canister@5 code_uninstall/canister "
        "load_snapshot(0001,2,metadata_upload)/user controllers_change(0)/user "
        "settings_change(1)/user rename_canister(aaaaa-aa->2vxsx-fae,1)/user",
        "canister_info: every kind of change");
    check_eq(
        mockIC.last_call_arg_hex("aaaaa-aa", "canister_info"),
        "4449444c026c02b3c4b1f20468f9bcd2e807016e780100010a00000000000000020101011400000000000000",
        "canister_info: the argument");
    CandidArgs in_all;
    in_all.append(CandidTypePrincipal{child});
    in_all.append(CandidTypeOptNat64{std::optional<uint64_t>{}});
    run_text(mockIC, "info_child all", info_child, in_all, caller);
    check_eq(
        mockIC.last_call_arg_hex("aaaaa-aa", "canister_info"),
        "4449444c026c02b3c4b1f20468f9bcd2e807016e780100010a0000000000000002010100",
        "canister_info: the argument, no count");
    mockIC.clear_expected_calls();
  }
  {
    // canister_info with a kind of change that icpp-pro does not know: traps
    mockIC.expect_call(
        "aaaaa-aa", "canister_info",
        MockIC::MockResponse::reply(
            "4449444c176c04d7e09b90020181cfaef40a029ff4c1b60b048fedd8b10e786d686e03"
            "6d7b6d056c04d6f68e800178c0c3dff50278e6b384d80406c2b9dbda0a0a6b0280d1e8"
            "900207dced83b40b086c018fc1d4fb06686c02c0c3dff50209b3c4b1f204686e786b08"
            "9f90dedf020bd798fbac040cbd81f4ce040e8cb1bb9a0511c89cb4c90a1398f795de0a"
            "15acd2c7a20c1690c6909a0f7f6c02d7e09b900201c2e7e2ce0b026c02e3a683c3040d"
            "81cfaef40a036b03c8bb8a707f9ce9c699067f9baaebec087f6c05c0c3dff50278dbe1"
            "c691030f9ed9c0ad081082bff3a50d78b6b897890f036b0291a0c58d0370a4f794e508"
            "706e686c02d7e09b900212c2e7e2ce0b026e016c04dca5dce70214b3c4b1f20468c8b6"
            "fa9307688fedd8b10e786c03b3c4b1f2046898cec7e707788fedd8b10e786c01d7e09b"
            "9002016c01787d01000000010100000000000000000000000000000000010104060101"
            "00000000000000"));
    CandidArgs in;
    in.append(CandidTypePrincipal{child});
    in.append(CandidTypeOptNat64{std::optional<uint64_t>{}});
    mockIC.run_trap_test("info_child future change", info_child, hex(in), true,
                         caller,
                         "does not match any of the expected Variant ids");
    mockIC.clear_expected_calls();
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
