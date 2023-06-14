#pragma once

#include "canister_base.h"
#include <string>

class CanisterQuery : public CanisterBase {
public:
  CanisterQuery();
  CanisterQuery(std::string calling_function);
  ~CanisterQuery();
};