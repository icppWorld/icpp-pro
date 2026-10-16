#include "canister.h"

CanisterCompositeCleanupCallback::CanisterCompositeCleanupCallback()
    : CanisterBase() {}

CanisterCompositeCleanupCallback::CanisterCompositeCleanupCallback(
    std::string calling_function)
    : CanisterBase(calling_function, std::string(__func__)) {}

CanisterCompositeCleanupCallback::~CanisterCompositeCleanupCallback() {}