// Exercises the environment-variable IC_APIs and the getenv() bridge. The
// deployed canister has the variables set in icp.yaml (settings), plus the
// PUBLIC_CANISTER_ID:<name> variables that icp-cli sets on deploy.

#include "my_canister.h"

#include <cstdlib>
#include <optional>
#include <string>

#include "ic_api.h"

// '("NAME")' -> '(IC_API::get_env_var("NAME"), getenv("NAME"))'
void test_env_var() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  std::string name;
  ic_api.from_wire(CandidTypeText{&name});

  std::optional<std::string> from_libc;
  if (const char *value = getenv(name.c_str())) from_libc = value;

  CandidArgs args_out;
  args_out.append(CandidTypeOptText{IC_API::get_env_var(name)});
  args_out.append(CandidTypeOptText{from_libc});
  ic_api.to_wire(args_out);
}

// '()' -> '(N : nat64)': the number of environment variables
void test_env_var_count() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  ic_api.to_wire(CandidTypeNat64{IC_API::get_env_vars().size()});
}
