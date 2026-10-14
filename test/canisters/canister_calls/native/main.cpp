// Main entry point for a native debug executable.
// Build it with: `icpp build-native` from the parent folder where 'icpp.toml'
// resides.
//
// The Mock IC delivers the calls a method makes after it returns: a call to
// the canister itself (MOCKIC_CANISTER_SELF) runs the registered method, a
// call to another canister gets the response set with expect_call.

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

// The weak default canister_pre_upgrade of ic_async.cpp
void icpp_default_pre_upgrade() WASM_SYMBOL_EXPORTED("canister_pre_upgrade");

namespace {
// Another canister, with programmed responses
const std::string CANISTER_B{"ryjl3-tyaaa-aaaaa-aaaba-cai"};

// Candid hex of the arguments: (principal, ...) with the canister itself or
// canister B
const std::string GREET_SELF{
    "4449444c00026871010a0000000000c0115a010103432b2b"}; // (self, "C++")
const std::string GREET_B{
    "4449444c00026871010a0000000000000002010103432b2b"}; // (B, "C++")
const std::string P_SELF{"4449444c000168010a0000000000c0115a0101"}; // (self)
const std::string P_B{"4449444c000168010a00000000000000020101"};    // (B)
const std::string CYCLES_SELF{
    "4449444c00026878010a0000000000c0115a010140420f0000000000"}; // (self, 1M)
const std::string BOUNDED_B{
    "4449444c0003687979010a000000000000000201010100000003000000"}; // (B,1,3)
const std::string N3{"4449444c00017903000000"};                    // (3)
const std::string N2{"4449444c00017902000000"};                    // (2)
const std::string EMPTY{"4449444c0000"};                           // ()

const std::string OUT_HELLO{"4449444c0001710b48656c6c6f2c20432b2b21"};
const std::string OUT_HI_B{"4449444c0001710948692066726f6d2042"};
const std::string OUT_CYCLES{
    "4449444c0002787820a107000000000020a1070000000000"}; // (500_000, 500_000)

// Runs a method that replies with text, and checks the text starts with
// `prefix`.
int run_text_test(MockIC &mockIC, const std::string &name, void (*fn)(),
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
  return 0;
}

// '()' -> trap while calls are in flight (the IC_API constructor path)
void native_pre_upgrade_with_ic_api() {
  IC_API ic_api(CanisterPreUpgrade{"native_pre_upgrade_with_ic_api"}, false);
}
} // namespace

int main() {
  bool exit_on_fail = true;
  MockIC mockIC(exit_on_fail);

  std::string my_principal{
      "expmt-gtxsw-inftj-ttabj-qhp5s-nozup-n3bbo-k7zvn-dg4he-knac3-lae"};
  bool silent_on_trap = true;

  // The methods a call to the canister itself can run
  mockIC.register_method("greet", greet);
  mockIC.register_method("reject_me", reject_me);
  mockIC.register_method("trap_me", trap_me);
  mockIC.register_method("accept_half", accept_half);
  mockIC.register_method("slow", slow);
  mockIC.register_method("hold", hold);

  // A call to the canister itself, and to canister B
  mockIC.run_test("call_greet self", call_greet, GREET_SELF, OUT_HELLO,
                  silent_on_trap, my_principal);
  mockIC.expect_call(CANISTER_B, "greet",
                     MockIC::MockResponse::reply(OUT_HI_B));
  mockIC.run_test("call_greet B", call_greet, GREET_B, OUT_HI_B, silent_on_trap,
                  my_principal);

  // Rejects: by the callee, by a trap in the callee, by the IC
  run_text_test(mockIC, "call_reject self", call_reject, P_SELF,
                "4:rejected on purpose", my_principal);
  run_text_test(mockIC, "call_reject B (no response programmed)", call_reject,
                P_B, "3:Mock IC: no response", my_principal);
  mockIC.expect_call(CANISTER_B, "reject_me",
                     MockIC::MockResponse::reject(2, "try again"));
  run_text_test(mockIC, "call_reject B (SYS_TRANSIENT)", call_reject, P_B,
                "2:try again", my_principal);
  run_text_test(mockIC, "call_trap self", call_trap, P_SELF, "5", my_principal);

  // A call that is not performed: the coroutine resumes with the error
  mockIC.set_next_call_perform_rc(2);
  run_text_test(mockIC, "call_reject, call_perform fails", call_reject, P_SELF,
                "0:", my_principal);
  __uint128_t balance = mockIC.get_canister_self_cycle_balance();
  mockIC.sub_cycles_balance(balance);
  run_text_test(mockIC, "call_reject, insufficient liquid balance", call_reject,
                P_SELF, "0:", my_principal);
  mockIC.add_cycles_balance(balance);

  // Cycles: half accepted, half refunded, the balance back where it was
  mockIC.run_test("call_with_cycles self", call_with_cycles, CYCLES_SELF,
                  OUT_CYCLES, silent_on_trap, my_principal);
  if (mockIC.get_canister_self_cycle_balance() != balance) {
    std::cout << "Test 'call_with_cycles self' Failed: balance changed\n";
    exit(1);
  }

  // A best-effort call that times out
  mockIC.expect_call(CANISTER_B, "slow", MockIC::MockResponse::timeout());
  run_text_test(mockIC, "call_bounded B (timeout)", call_bounded, BOUNDED_B,
                "6:", my_principal);

  // A best-effort call to the canister itself: slow() checks it sees a
  // deadline
  run_text_test(mockIC, "call_bounded self", call_bounded,
                "4449444c0003687979010a0000000000c0115a01013c00000003000000",
                "replied", my_principal);

  // A oneway call: performed, no response
  mockIC.run_test("call_oneway self", call_oneway, P_SELF, "4449444c00017e01",
                  silent_on_trap, my_principal);

  // Yields: the caller survives them, each resumes in a new message
  run_text_test(mockIC, "yield_loop 3", yield_loop, N3,
                my_principal + " after 3 yields", my_principal);
  mockIC.run_test("budget_probe 2", budget_probe, N2, "", silent_on_trap,
                  my_principal);

  // The code after a co_await traps: the cleanup frees the frame, and the
  // call is no longer in flight. (The Mock IC does not roll back g_after.)
  mockIC.run_test("call_then_trap self", call_then_trap, P_SELF, "",
                  silent_on_trap, my_principal);
  // (before, after, destroyed, calls_in_flight, held_done)
  mockIC.run_test("get_state after call_then_trap", get_state, EMPTY,
                  "4449444c0005787878797e01000000000000000100000000000000"
                  "01000000000000000000000000",
                  silent_on_trap, my_principal);

  // An upgrade while a call is in flight is refused
  mockIC.set_auto_drain(false);
  mockIC.run_test("start_held_call self", start_held_call, P_SELF,
                  "4449444c0000", silent_on_trap, my_principal);
  mockIC.run_trap_test("canister_pre_upgrade (default), call in flight",
                       icpp_default_pre_upgrade, EMPTY, silent_on_trap,
                       my_principal);
  mockIC.run_trap_test("canister_pre_upgrade (IC_API), call in flight",
                       native_pre_upgrade_with_ic_api, EMPTY, silent_on_trap,
                       my_principal);
  mockIC.run_test("release", release, EMPTY, "4449444c0000", silent_on_trap,
                  my_principal);
  mockIC.drain(); // hold() sees the release and returns
  mockIC.set_auto_drain(true);
  mockIC.run_test("canister_pre_upgrade (default), no call in flight",
                  icpp_default_pre_upgrade, EMPTY, "", silent_on_trap,
                  my_principal);
  mockIC.run_test("get_state after the held call", get_state, EMPTY,
                  "4449444c0005787878797e01000000000000000100000000000000"
                  "01000000000000000000000001",
                  silent_on_trap, my_principal);

  // returns 1 if any tests failed
  return mockIC.test_summary();
}
