// The base class for all Canister Entry points

#pragma once

#include <string>

#include "canister.h"

class CanisterBase {
public:
  CanisterBase();
  CanisterBase(std::string calling_function);
  ~CanisterBase();

protected:
  // The calling function
  std::string m_calling_function{""};
};