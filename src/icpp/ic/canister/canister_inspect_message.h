#pragma once

#include "canister_base.h"
#include <string>

class CanisterInspectMessage : public CanisterBase {
public:
  CanisterInspectMessage();
  CanisterInspectMessage(std::string calling_function);
  ~CanisterInspectMessage();
};