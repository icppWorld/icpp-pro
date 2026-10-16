#pragma once

#include "canister_base.h"
#include <string>

class CanisterCompositeCleanupCallback : public CanisterBase {
public:
  CanisterCompositeCleanupCallback();
  CanisterCompositeCleanupCallback(std::string calling_function);
  ~CanisterCompositeCleanupCallback();
};