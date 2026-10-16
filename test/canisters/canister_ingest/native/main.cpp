// Main entry point for a native debug executable.
// Build it with: `icpp build-native` from the parent folder where 'icpp.toml'
// resides.
//
// The Mock IC delivers the self-calls after the method returns, to the
// methods registered with register_method. Natively, the instruction counter
// counts nanoseconds.

#include "main.h"

#include <iostream>
#include <string>

#include "../src/canister_ingest.h"

// The Mock IC
// clang-format off
#include "ic_api.h"
#include "icpp_hooks.h"
#include "mock_ic.h"
#include "ic0.h"
// clang-format on

// The weak default canister_pre_upgrade of ic_async.cpp
void icpp_default_pre_upgrade() WASM_SYMBOL_EXPORTED("canister_pre_upgrade");

namespace {
const std::string EMPTY{"4449444c0000"}; // ()
// (vec {"apple pie"; "apple tart"; "cherry"})
const std::string MESSAGES{"4449444c016d71010003096170706c65207069650a6170706c"
                           "65207461727406636865727279"};
const std::string BYTES_28{"4449444c0001781c00000000000000"}; // (28 : nat64)
const std::string APPLE{"4449444c000171056170706c65"};        // ("apple")
// (2 : nat32, 1_000_000 : nat64)
const std::string BURN_2{"4449444c000279780200000040420f0000000000"};

// Runs a method that replies with text, and checks the text starts with
// `prefix`.
void run_text_test(MockIC &mockIC, const std::string &name, void (*fn)(),
                   const std::string &in, const std::string &prefix,
                   const std::string &caller) {
  std::string out_hex;
  mockIC.run_test(name, fn, in, "", true, caller, &out_hex);
  VecBytes out;
  out.store_hex_string(out_hex);
  std::string text;
  CandidArgs args;
  args.append(CandidTypeText{&text});
  CandidDeserialize(out, args);
  if (text.rfind(prefix, 0) != 0) {
    std::cout << "Test '" << name << "' Failed: '" << text
              << "' does not start with '" << prefix << "'\n";
    exit(1);
  }
  std::cout << "Test '" << name << "': reply '" << text << "' - Passed\n";
}
} // namespace

int main() {
  bool exit_on_fail = true;
  MockIC mockIC(exit_on_fail);

  std::string my_principal{
      "expmt-gtxsw-inftj-ttabj-qhp5s-nozup-n3bbo-k7zvn-dg4he-knac3-lae"};
  bool silent_on_trap = true;

  // The methods a self-call can run
  mockIC.register_method("store", store);
  mockIC.register_method("ask", ask, MockIC::MethodKind::Query);
  mockIC.register_method("burn_q", burn_q, MockIC::MethodKind::Query);

  // Ingest three messages, with an update self-call each
  mockIC.run_test("reset", reset, EMPTY, EMPTY, silent_on_trap, my_principal);
  mockIC.run_test("ingest", ingest, MESSAGES, BYTES_28, silent_on_trap,
                  my_principal);

  // Ask about them, with a query and with a composite query
  run_text_test(mockIC, "ask", ask, APPLE, "3 messages, 2 mention apple",
                my_principal);
  run_text_test(mockIC, "ask_cq", ask_cq, APPLE,
                "via ask: 3 messages, 2 mention apple", my_principal);

  // A composite query that calls a query of itself twice
  run_text_test(mockIC, "cq_burn", cq_burn, BURN_2, "parts: 2 of 2",
                my_principal);

  mockIC.run_test("reset again", reset, EMPTY, EMPTY, silent_on_trap,
                  my_principal);

  // returns 1 if any tests failed
  return mockIC.test_summary();
}
