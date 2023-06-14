#include "canister.h"

CanisterPostUpgrade::CanisterPostUpgrade() : CanisterBase() {}

CanisterPostUpgrade::CanisterPostUpgrade(std::string calling_function)
    : CanisterBase(calling_function) {}

CanisterPostUpgrade::~CanisterPostUpgrade() {}