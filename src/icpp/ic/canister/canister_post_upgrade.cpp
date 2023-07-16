#include "canister.h"

CanisterPostUpgrade::CanisterPostUpgrade() : CanisterBase() {}

CanisterPostUpgrade::CanisterPostUpgrade(std::string calling_function)
    : CanisterBase(calling_function, std::string(__func__)) {}

CanisterPostUpgrade::~CanisterPostUpgrade() {}