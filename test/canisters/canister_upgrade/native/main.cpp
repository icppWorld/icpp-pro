// Main entry point for a native debug executable.
// Build it with: `icpp build-native` from the parent folder where 'icpp.toml'
// resides.
//
// MockIC cannot install or upgrade a canister, so this driver calls the
// lifecycle hooks directly in the order the replica would - init, then
// pre_upgrade & post_upgrade for an upgrade - and asserts the record after
// each phase. The Mock IC refuses msg_arg_data in pre_upgrade, like the
// replica, so a regression of the IC_API constructor fails here natively.

#include "main.h"

#include <cstdio>
#include <string>

#include "../src/my_canister.h"
#include "../src/upgrade_record.h"

// The Mock IC
#include "ic_api.h"
#include "icpp_hooks.h"
#include "mock_ic.h"
#include "mock_ic_constants.h"

namespace {
const std::string MY_PRINCIPAL{
    "expmt-gtxsw-inftj-ttabj-qhp5s-nozup-n3bbo-k7zvn-dg4he-knac3-lae"};

void expect(bool condition, const std::string &what) {
  if (!condition) ICPP_HOOKS::trap("expected: " + what);
}
} // namespace

// After a fresh install: only canister_init ran
static void native_record_after_install() {
  const UpgradeRecord r = load_record();
  expect(r.init_count == 1, "init_count == 1");
  expect(r.pre_upgrade_count == 0, "pre_upgrade_count == 0");
  expect(r.post_upgrade_count == 0, "post_upgrade_count == 0");
  expect(r.init_caller == MY_PRINCIPAL, "init_caller == my principal");
  expect(r.init_self == MOCKIC_CANISTER_SELF, "init_self == canister self");
}

// After an upgrade: both upgrade hooks ran once, init did not run again
static void native_record_after_upgrade() {
  const UpgradeRecord r = load_record();
  expect(r.init_count == 1, "init_count == 1");
  expect(r.pre_upgrade_count == 1, "pre_upgrade_count == 1");
  expect(r.post_upgrade_count == 1, "post_upgrade_count == 1");
  expect(r.pre_upgrade_caller == MY_PRINCIPAL,
         "pre_upgrade_caller == my principal");
  expect(r.post_upgrade_caller == MY_PRINCIPAL,
         "post_upgrade_caller == my principal");
  expect(r.pre_upgrade_self == MOCKIC_CANISTER_SELF,
         "pre_upgrade_self == canister self");
  expect(r.post_upgrade_self == MOCKIC_CANISTER_SELF,
         "post_upgrade_self == canister self");
}

int main() {
  bool exit_on_fail = true;
  bool silent_on_trap = true;
  MockIC mockIC(exit_on_fail);

  // Start from an uninstalled canister
  std::remove(UPGRADE_RECORD_PATH);

  // '()' -> the argument of an install or upgrade without one
  const std::string no_args{"4449444c0000"};

  // Install
  mockIC.run_test("canister_init", canister_init, no_args, "", silent_on_trap,
                  MY_PRINCIPAL);
  mockIC.run_test("native_record_after_install", native_record_after_install,
                  no_args, "", silent_on_trap, MY_PRINCIPAL);
  mockIC.run_test("get_upgrade_record", get_upgrade_record, no_args, "",
                  silent_on_trap, MY_PRINCIPAL);

  // Upgrade
  mockIC.run_test("canister_pre_upgrade", canister_pre_upgrade, no_args, "",
                  silent_on_trap, MY_PRINCIPAL);
  mockIC.run_test("canister_post_upgrade", canister_post_upgrade, no_args, "",
                  silent_on_trap, MY_PRINCIPAL);
  mockIC.run_test("native_record_after_upgrade", native_record_after_upgrade,
                  no_args, "", silent_on_trap, MY_PRINCIPAL);
  mockIC.run_test("get_upgrade_record", get_upgrade_record, no_args, "",
                  silent_on_trap, MY_PRINCIPAL);

  // Leave no state behind in the working directory
  std::remove(UPGRADE_RECORD_PATH);

  // returns 1 if any tests failed
  return mockIC.test_summary();
}
