#pragma once

#include "canister_base.h"
#include <string>

class CanisterPostUpgrade : public CanisterBase {
public:
  CanisterPostUpgrade();
  CanisterPostUpgrade(std::string calling_function);
  ~CanisterPostUpgrade();
};