#pragma once

#include "canister_base.h"
#include <string>

class CanisterReplyCallback : public CanisterBase {
public:
  CanisterReplyCallback();
  CanisterReplyCallback(std::string calling_function);
  ~CanisterReplyCallback();
};