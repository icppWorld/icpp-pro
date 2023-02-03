// Candid Assert helper class

#include "candid_assert.h"

#include "ic_api.h"

CandidAssert::CandidAssert() {}
CandidAssert::~CandidAssert() {}

// Assert a candid VecBytes against a string in "hex" format (didc encode)
int CandidAssert::assert_candid(const VecBytes &B,
                                const std::string &candid_expected,
                                const bool &assert_value) {
  std::string s_hex;

  for (std::byte b : B.vec()) {
    s_hex.append(VecBytes::byte_to_hex(b, ""));
  }

  if (candid_expected == s_hex) {
    if (assert_value) return 0;
    return 1;
  } else {
    if (assert_value) {
      IC_API::debug_print("FAIL - assert_candid");
      IC_API::debug_print("Expecting:");
      IC_API::debug_print(candid_expected);
      IC_API::debug_print("Found:");
      IC_API::debug_print(s_hex);
      IC_API::debug_print("To debug, you can decode it with:");
      IC_API::debug_print("didc decode " + s_hex);
      return 1;
    }
    return 0;
  }
}
