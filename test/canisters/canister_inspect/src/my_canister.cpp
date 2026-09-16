// Ingress message inspection: accept only methods whose name starts with
// "allowed_". Every other ingress update call is refused before execution.

#include "my_canister.h"

#include <string>

#include "ic_api.h"

void canister_inspect_message() {
  IC_API ic_api(CanisterInspectMessage{std::string(__func__)}, false);
  std::string method_name = ic_api.get_msg_method_name();
  if (method_name.rfind("allowed_", 0) == 0) {
    ic_api.accept_message();
  }
  // Not calling accept_message rejects the ingress message.
}

// '()' -> '("Hello!" : text)'
void allowed_greet() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  ic_api.from_wire();
  ic_api.to_wire(CandidTypeText{"Hello!"});
}

// Identical to allowed_greet - but its name fails inspection, so an ingress
// call never reaches it.
void blocked_greet() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  ic_api.from_wire();
  ic_api.to_wire(CandidTypeText{"Hello!"});
}
