#include "canister.h"

CanisterCleanupCallback::CanisterCleanupCallback() : CanisterBase() {}

CanisterCleanupCallback::CanisterCleanupCallback(std::string calling_function)
    : CanisterBase(calling_function, std::string(__func__)) {}

CanisterCleanupCallback::~CanisterCleanupCallback() {}