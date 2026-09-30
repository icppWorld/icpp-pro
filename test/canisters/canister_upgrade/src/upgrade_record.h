#pragma once

#include <cstdint>
#include <string>

// What the lifecycle hooks record about each run. It lives in a file because
// the Wasm heap is wiped by an upgrade, while files survive it: ic-wasi-polyfill
// keeps them in stable memory. Raw stable memory belongs to the polyfill, so
// the canister never touches it directly.
struct UpgradeRecord {
  uint64_t init_count{0};
  uint64_t pre_upgrade_count{0};
  uint64_t post_upgrade_count{0};
  std::string init_caller;
  std::string pre_upgrade_caller;
  std::string post_upgrade_caller;
  std::string init_self;
  std::string pre_upgrade_self;
  std::string post_upgrade_self;
};

inline constexpr const char *UPGRADE_RECORD_PATH = "upgrade_record.txt";

// All zeros & empty strings when the file does not exist yet.
UpgradeRecord load_record();

// Traps when the file cannot be written: a hook that silently lost its record
// would make the tests blame the upgrade instead of the file system.
void save_record(const UpgradeRecord &record);
