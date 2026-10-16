#include "canister.h"

CanisterCompositeReplyCallback::CanisterCompositeReplyCallback()
    : CanisterBase() {}

CanisterCompositeReplyCallback::CanisterCompositeReplyCallback(
    std::string calling_function)
    : CanisterBase(calling_function, std::string(__func__)) {}

CanisterCompositeReplyCallback::~CanisterCompositeReplyCallback() {}