#include "canister.h"

CanisterUpdate::CanisterUpdate() : CanisterBase() {}

CanisterUpdate::CanisterUpdate(std::string calling_function)
    : CanisterBase(calling_function, std::string(__func__)) {}

CanisterUpdate::~CanisterUpdate() {}