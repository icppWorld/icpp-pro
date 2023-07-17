#include "canister.h"

CanisterInit::CanisterInit() : CanisterBase() {}

CanisterInit::CanisterInit(std::string calling_function)
    : CanisterBase(calling_function, std::string(__func__)) {}

CanisterInit::~CanisterInit() {}