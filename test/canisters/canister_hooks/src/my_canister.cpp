#include "my_canister.h"

#include <string>

#include "ic_api.h"

void greet() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  ic_api.to_wire(CandidTypeText{"Hello World"});
}