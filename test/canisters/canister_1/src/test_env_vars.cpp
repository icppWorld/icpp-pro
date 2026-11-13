// Exercises the environment-variable IC_APIs and the getenv() bridge. The
// deployed canister has the variables set in icp.yaml (settings), plus the
// PUBLIC_CANISTER_ID:<name> variables that icp-cli sets on deploy.

#include "my_canister.h"

#include <cstdlib>
#include <optional>
#include <string>
#include <vector>

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

#if defined(__wasm__)
extern "C" void icpp_environ_set_available(int available);
#endif

// '()' -> '(getenv("ICPP_TEST_GREETING"))', read through a pointer taken
// before an entry-point change, after the heap was reused
void test_getenv_kept() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  const char *kept = getenv("ICPP_TEST_GREETING");
#if defined(__wasm__)
  // What IC_API does at every co_await and at the end of every message
  icpp_environ_set_available(1);
  std::vector<std::string> fill;
  for (size_t n = 16; n <= 4096; n += 16)
    fill.emplace_back(n, 'X');
#endif
  ic_api.to_wire(CandidTypeText{kept != nullptr ? kept : ""});
}

// '()' -> '(getenv("ICPP_TEST_GREETING"))' after setenv() changed it. The
// change lasts for this message only
void test_setenv() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  setenv("ICPP_TEST_GREETING", "changed by setenv", 1);
  const char *value = getenv("ICPP_TEST_GREETING");
  ic_api.to_wire(CandidTypeText{value != nullptr ? value : ""});
}
