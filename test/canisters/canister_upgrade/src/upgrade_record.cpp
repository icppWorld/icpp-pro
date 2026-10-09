#include "upgrade_record.h"

#include <fstream>
#include <map>

#include "ic_api.h"

UpgradeRecord load_record() {
  UpgradeRecord record;
  std::ifstream file(UPGRADE_RECORD_PATH);
  if (!file) return record;

  // One `key=value` per line
  std::map<std::string, std::string> kv;
  std::string line;
  while (std::getline(file, line)) {
    const auto pos = line.find('=');
    if (pos != std::string::npos)
      kv[line.substr(0, pos)] = line.substr(pos + 1);
  }

  auto count = [&kv](const std::string &key) -> uint64_t {
    return kv.count(key) ? std::stoull(kv[key]) : 0;
  };
  record.init_count = count("init_count");
  record.pre_upgrade_count = count("pre_upgrade_count");
  record.post_upgrade_count = count("post_upgrade_count");
  record.init_caller = kv["init_caller"];
  record.pre_upgrade_caller = kv["pre_upgrade_caller"];
  record.post_upgrade_caller = kv["post_upgrade_caller"];
  record.init_self = kv["init_self"];
  record.pre_upgrade_self = kv["pre_upgrade_self"];
  record.post_upgrade_self = kv["post_upgrade_self"];
  return record;
}

void save_record(const UpgradeRecord &record) {
  std::ofstream file(UPGRADE_RECORD_PATH, std::ios::trunc);
  file << "init_count=" << record.init_count << "\n"
       << "pre_upgrade_count=" << record.pre_upgrade_count << "\n"
       << "post_upgrade_count=" << record.post_upgrade_count << "\n"
       << "init_caller=" << record.init_caller << "\n"
       << "pre_upgrade_caller=" << record.pre_upgrade_caller << "\n"
       << "post_upgrade_caller=" << record.post_upgrade_caller << "\n"
       << "init_self=" << record.init_self << "\n"
       << "pre_upgrade_self=" << record.pre_upgrade_self << "\n"
       << "post_upgrade_self=" << record.post_upgrade_self << "\n";
  file.close();
  if (!file)
    IC_API::trap(std::string("Failed to write ") + UPGRADE_RECORD_PATH);
}
