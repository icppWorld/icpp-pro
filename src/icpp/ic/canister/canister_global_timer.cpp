#include "canister.h"

CanisterGlobalTimer::CanisterGlobalTimer() : CanisterBase() {}

CanisterGlobalTimer::CanisterGlobalTimer(std::string calling_function)
    : CanisterBase(calling_function, std::string(__func__)) {}

CanisterGlobalTimer::~CanisterGlobalTimer() {}