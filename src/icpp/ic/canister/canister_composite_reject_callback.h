#pragma once

#include "canister_base.h"
#include <string>

class CanisterCompositeRejectCallback : public CanisterBase {
public:
  CanisterCompositeRejectCallback();
  CanisterCompositeRejectCallback(std::string calling_function);
  ~CanisterCompositeRejectCallback();
};