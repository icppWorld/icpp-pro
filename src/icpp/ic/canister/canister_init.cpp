#include "canister.h"

// C++ global constructors are run once per wasm instance by
// icpp_run_global_ctors_once(), invoked from the CanisterBase constructor
// (see canister_base.cpp). That covers every entry type — init,
// post_upgrade, update, query, and the reply/reject callbacks — so a wasm
// instance created by an upgrade (where canister_init never runs) still
// gets its globals constructed at the first entry.

CanisterInit::CanisterInit() : CanisterBase() {}

CanisterInit::CanisterInit(std::string calling_function)
    : CanisterBase(calling_function, std::string(__func__)) {}

CanisterInit::~CanisterInit() {}