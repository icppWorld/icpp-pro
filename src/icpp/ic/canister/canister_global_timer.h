#pragma once

#include "canister_base.h"
#include <string>

class CanisterGlobalTimer : public CanisterBase {
public:
  CanisterGlobalTimer();
  CanisterGlobalTimer(std::string calling_function);
  ~CanisterGlobalTimer();
};