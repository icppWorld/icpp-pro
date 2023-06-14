#pragma once

#include "canister_base.h"
#include <string>

class CanisterHeartbeat : public CanisterBase {
public:
  CanisterHeartbeat();
  CanisterHeartbeat(std::string calling_function);
  ~CanisterHeartbeat();
};