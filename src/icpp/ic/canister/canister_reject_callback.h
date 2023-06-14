#pragma once

#include "canister_base.h"
#include <string>

class CanisterRejectCallback : public CanisterBase {
public:
  CanisterRejectCallback();
  CanisterRejectCallback(std::string calling_function);
  ~CanisterRejectCallback();
};