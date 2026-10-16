#include "canister.h"

CanisterCompositeRejectCallback::CanisterCompositeRejectCallback()
    : CanisterBase() {}

CanisterCompositeRejectCallback::CanisterCompositeRejectCallback(
    std::string calling_function)
    : CanisterBase(calling_function, std::string(__func__)) {}

CanisterCompositeRejectCallback::~CanisterCompositeRejectCallback() {}