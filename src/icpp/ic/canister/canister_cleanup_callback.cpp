#include "canister.h"

CanisterCleanupCallback::CanisterCleanupCallback() : CanisterBase() {}

CanisterCleanupCallback::CanisterCleanupCallback(std::string calling_function)
    : CanisterBase(calling_function) {}

CanisterCleanupCallback::~CanisterCleanupCallback() {}