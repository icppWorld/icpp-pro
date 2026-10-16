#pragma once

#include "canister_base.h"
#include <string>

class CanisterCompositeReplyCallback : public CanisterBase {
public:
  CanisterCompositeReplyCallback();
  CanisterCompositeReplyCallback(std::string calling_function);
  ~CanisterCompositeReplyCallback();
};