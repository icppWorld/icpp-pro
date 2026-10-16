#include "canister.h"

CanisterCompositeQuery::CanisterCompositeQuery() : CanisterBase() {}

CanisterCompositeQuery::CanisterCompositeQuery(std::string calling_function)
    : CanisterBase(calling_function, std::string(__func__)) {}

CanisterCompositeQuery::~CanisterCompositeQuery() {}