#include "canister.h"

CanisterInspectMessage::CanisterInspectMessage() : CanisterBase() {}

CanisterInspectMessage::CanisterInspectMessage(std::string calling_function)
    : CanisterBase(calling_function, std::string(__func__)) {}

CanisterInspectMessage::~CanisterInspectMessage() {}