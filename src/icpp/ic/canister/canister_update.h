#pragma once

#include "canister_base.h"
#include <string>

class CanisterUpdate : public CanisterBase {
public:
  CanisterUpdate();
  CanisterUpdate(std::string calling_function);
  ~CanisterUpdate();
};