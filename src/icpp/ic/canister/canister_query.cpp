#include "canister.h"

CanisterQuery::CanisterQuery() : CanisterBase() {}

CanisterQuery::CanisterQuery(std::string calling_function)
    : CanisterBase(calling_function, std::string(__func__)) {}

CanisterQuery::~CanisterQuery() {}