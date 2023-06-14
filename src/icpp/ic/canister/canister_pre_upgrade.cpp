#include "canister.h"

CanisterPreUpgrade::CanisterPreUpgrade() : CanisterBase() {}

CanisterPreUpgrade::CanisterPreUpgrade(std::string calling_function)
    : CanisterBase(calling_function) {}

CanisterPreUpgrade::~CanisterPreUpgrade() {}