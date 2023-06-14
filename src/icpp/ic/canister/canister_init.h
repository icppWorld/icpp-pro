#pragma once

#include "canister_base.h"
#include <string>

class CanisterInit : public CanisterBase {
public:
  CanisterInit();
  CanisterInit(std::string calling_function);
  ~CanisterInit();
};