#pragma once

#include "canister_base.h"
#include <string>

class CanisterCompositeQuery : public CanisterBase {
public:
  CanisterCompositeQuery();
  CanisterCompositeQuery(std::string calling_function);
  ~CanisterCompositeQuery();
};