#include "canister.h"

CanisterHeartbeat::CanisterHeartbeat() : CanisterBase() {}

CanisterHeartbeat::CanisterHeartbeat(std::string calling_function)
    : CanisterBase(calling_function, std::string(__func__)) {}

CanisterHeartbeat::~CanisterHeartbeat() {}