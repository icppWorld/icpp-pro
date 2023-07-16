#include "canister.h"

CanisterRejectCallback::CanisterRejectCallback() : CanisterBase() {}

CanisterRejectCallback::CanisterRejectCallback(std::string calling_function)
    : CanisterBase(calling_function, std::string(__func__)) {}

CanisterRejectCallback::~CanisterRejectCallback() {}