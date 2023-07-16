#include "canister.h"

CanisterStart::CanisterStart() : CanisterBase() {}

CanisterStart::CanisterStart(std::string calling_function)
    : CanisterBase(calling_function, std::string(__func__)) {}

CanisterStart::~CanisterStart() {}