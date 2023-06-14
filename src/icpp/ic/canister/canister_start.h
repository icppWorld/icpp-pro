#pragma once

#include "canister_base.h"
#include <string>

class CanisterStart : public CanisterBase {
public:
  CanisterStart();
  CanisterStart(std::string calling_function);
  ~CanisterStart();
};