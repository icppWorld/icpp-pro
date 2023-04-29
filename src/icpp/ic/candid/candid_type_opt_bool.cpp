// The class for the Candid Type: opt

#include "candid.h"
#include "candid_opcode.h"

#include <cassert>

#include "ic_api.h"

CandidTypeOptBool::CandidTypeOptBool() : CandidTypeOptBase() {}

CandidTypeOptBool::CandidTypeOptBool(std::optional<bool> *p_v)
    : CandidTypeOptBase() {}

CandidTypeOptBool::CandidTypeOptBool(const std::optional<bool> v)
    : CandidTypeOptBase() {}

CandidTypeOptBool::~CandidTypeOptBool() {}

void CandidTypeOptBool::set_content_type() {}

void CandidTypeOptBool::encode_M() {}

bool CandidTypeOptBool::decode_M(VecBytes B, __uint128_t &offset,
                                 std::string &parse_error) {
  return false;
}