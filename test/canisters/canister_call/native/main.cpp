// Native test driver for canister_call — placeholder.
// Inter-canister calls require a live replica (the Mock IC has no second
// canister to dispatch to), so this binary is intentionally a no-op. The
// behavioural tests live in test/test_apis.py and run against a real local
// replica via `dfx canister call`.

#include <iostream>

int main() {
  std::cout
      << "canister_call: native build is a no-op.\n"
      << "Run `pytest --network=local` after `dfx deploy` to exercise the "
         "ic_call.h inter-canister helper end-to-end.\n";
  return 0;
}
