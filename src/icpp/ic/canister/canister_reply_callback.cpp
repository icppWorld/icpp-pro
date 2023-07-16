#include "canister.h"

CanisterReplyCallback::CanisterReplyCallback() : CanisterBase() {}

CanisterReplyCallback::CanisterReplyCallback(std::string calling_function)
    : CanisterBase(calling_function, std::string(__func__)) {}

CanisterReplyCallback::~CanisterReplyCallback() {}