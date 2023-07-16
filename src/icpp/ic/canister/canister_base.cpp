// The base class for all Canister Entry points

#include "canister.h"

#include "candid_type.h"
#include "candid_type_all_includes.h"
#include "ic_api.h"

CanisterBase::CanisterBase() {}

CanisterBase::CanisterBase(std::string calling_function,
                           std::string entry_type) {
  m_calling_function = calling_function;
  m_entry_type = entry_type;
}

CanisterBase::~CanisterBase() {}
