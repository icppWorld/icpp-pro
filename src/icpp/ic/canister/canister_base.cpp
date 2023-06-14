// The base class for all Canister Entry points

#include "canister.h"

#include "ic_api.h"

CanisterBase::CanisterBase() {}

CanisterBase::CanisterBase(std::string calling_function) {
  m_calling_function = calling_function;
}

CanisterBase::~CanisterBase() {}
