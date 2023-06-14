#include "canister.h"

CanisterReplyCallback::CanisterReplyCallback() : CanisterBase() {}

CanisterReplyCallback::CanisterReplyCallback(std::string calling_function)
    : CanisterBase(calling_function) {}

CanisterReplyCallback::~CanisterReplyCallback() {}