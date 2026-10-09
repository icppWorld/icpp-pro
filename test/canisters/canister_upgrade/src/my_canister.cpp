// A canister that exports canister_init, canister_pre_upgrade and
// canister_post_upgrade, each constructing an IC_API for its own entry point.
//
// It guards a critical fix (icpp-pro 6.1.0): the IC_API constructor used to
// read msg_arg_data in every entry point, and the replica refuses that inside
// canister_pre_upgrade. The trap aborted the upgrade, so any canister that
// constructed an IC_API in pre_upgrade could never be upgraded again.
//
// Each hook counts its runs and records the caller & canister id it saw; the
// tests assert those after a fresh install and again after an upgrade.

#include "my_canister.h"

#include <string>

#include "ic_api.h"
#include "upgrade_record.h"

void canister_init() {
  IC_API ic_api(CanisterInit{std::string(__func__)}, false);
  UpgradeRecord record = load_record();
  record.init_count += 1;
  record.init_caller = ic_api.get_caller().get_text();
  record.init_self = ic_api.get_canister_self().get_text();
  save_record(record);
}

void canister_pre_upgrade() {
  IC_API ic_api(CanisterPreUpgrade{std::string(__func__)}, false);
  UpgradeRecord record = load_record();
  record.pre_upgrade_count += 1;
  record.pre_upgrade_caller = ic_api.get_caller().get_text();
  record.pre_upgrade_self = ic_api.get_canister_self().get_text();
  save_record(record);
}

void canister_post_upgrade() {
  IC_API ic_api(CanisterPostUpgrade{std::string(__func__)}, false);
  UpgradeRecord record = load_record();
  record.post_upgrade_count += 1;
  record.post_upgrade_caller = ic_api.get_caller().get_text();
  record.post_upgrade_self = ic_api.get_canister_self().get_text();
  save_record(record);
}

void get_upgrade_record() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  const UpgradeRecord record = load_record();

  CandidTypeRecord r_out;
  r_out.append("init_count", CandidTypeNat64{record.init_count});
  r_out.append("pre_upgrade_count", CandidTypeNat64{record.pre_upgrade_count});
  r_out.append("post_upgrade_count",
               CandidTypeNat64{record.post_upgrade_count});
  r_out.append("init_caller", CandidTypeText{record.init_caller});
  r_out.append("pre_upgrade_caller", CandidTypeText{record.pre_upgrade_caller});
  r_out.append("post_upgrade_caller",
               CandidTypeText{record.post_upgrade_caller});
  r_out.append("init_self", CandidTypeText{record.init_self});
  r_out.append("pre_upgrade_self", CandidTypeText{record.pre_upgrade_self});
  r_out.append("post_upgrade_self", CandidTypeText{record.post_upgrade_self});
  ic_api.to_wire(r_out);
}
