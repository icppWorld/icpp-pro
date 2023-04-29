// The class for the Candid Type: opt

#include "candid.h"
#include "candid_opcode.h"

#include <cassert>

#include "ic_api.h"

CandidTypeOptPrincipal::CandidTypeOptPrincipal() : CandidTypeOptBase() {}

CandidTypeOptPrincipal::CandidTypeOptPrincipal(std::optional<std::string> *p_v)
    : CandidTypeOptBase() {}

CandidTypeOptPrincipal::CandidTypeOptPrincipal(
    const std::optional<std::string> v)
    : CandidTypeOptBase() {}

CandidTypeOptPrincipal::~CandidTypeOptPrincipal() {}

void CandidTypeOptPrincipal::set_content_type() {}

void CandidTypeOptPrincipal::encode_M() {}

// Decode the values, starting at & updating offset
bool CandidTypeOptPrincipal::decode_M(VecBytes B, __uint128_t &offset,
                                      std::string &parse_error) {
  return false;
}