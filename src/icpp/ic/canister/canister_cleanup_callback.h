#pragma once

#include "canister_base.h"
#include <string>

class CanisterCleanupCallback : public CanisterBase {
public:
  CanisterCleanupCallback();
  CanisterCleanupCallback(std::string calling_function);
  ~CanisterCleanupCallback();
};