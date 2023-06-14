#pragma once

#include "canister_base.h"
#include <string>

class CanisterPreUpgrade : public CanisterBase {
public:
  CanisterPreUpgrade();
  CanisterPreUpgrade(std::string calling_function);
  ~CanisterPreUpgrade();
};