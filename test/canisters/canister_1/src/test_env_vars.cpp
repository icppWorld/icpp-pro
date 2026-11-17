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
  if (kept == nullptr)
    IC_API::trap("ASSERT ERROR - " + std::string(__func__) +
                 ": ICPP_TEST_GREETING is not set");
  // Two fills of 256 strings, reserved up front so that growing the vector
  // frees no buffer of its own
  constexpr size_t fill_min = 16, fill_max = 4096, fill_step = 16;
  constexpr size_t fill_count = (fill_max - fill_min) / fill_step + 1;
  std::vector<std::string> fill;
  fill.reserve(2 * fill_count);
  auto fill_heap = [&fill] {
    for (size_t n = fill_min; n <= fill_max; n += fill_step)
      fill.emplace_back(n, 'X');
    // Nothing reads the strings: keep the optimizer from dropping them
    asm volatile("" : : "r"(fill.data()) : "memory");
  };
  // What IC_API does at the end of a message and at the start of the next
  icpp_environ_set_available(0);
  icpp_environ_set_available(1);
  // Overwrites the block if the reset freed it
  fill_heap();
  // Rebuilds environ with the same variables, which must reuse the old block
  const char *rebuilt = getenv("ICPP_TEST_GREETING");
  if (rebuilt == nullptr)
    IC_API::trap("ASSERT ERROR - " + std::string(__func__) +
                 ": ICPP_TEST_GREETING is gone after the rebuild");
  if (rebuilt != kept)
    IC_API::trap("ASSERT ERROR - " + std::string(__func__) +
                 ": the environ block was not reused");
  // Overwrites the block if the rebuild kept its address but freed it, which
  // the pointer check above cannot see
  fill_heap();
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
