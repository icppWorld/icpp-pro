#include "canister.h"

CanisterQuery::CanisterQuery() : CanisterBase() {}

CanisterQuery::CanisterQuery(std::string calling_function)
    : CanisterBase(calling_function) {}

CanisterQuery::~CanisterQuery() {}