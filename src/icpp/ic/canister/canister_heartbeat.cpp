#include "canister.h"

CanisterHeartbeat::CanisterHeartbeat() : CanisterBase() {}

CanisterHeartbeat::CanisterHeartbeat(std::string calling_function)
    : CanisterBase(calling_function) {}

CanisterHeartbeat::~CanisterHeartbeat() {}