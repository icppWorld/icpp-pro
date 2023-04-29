// The class for the Candid Type: vec

#include "candid.h"
#include "candid_opcode.h"

#include <cassert>

#include "ic_api.h"

CandidTypeVecBool::CandidTypeVecBool() : CandidTypeVecBase() {}

CandidTypeVecBool::CandidTypeVecBool(std::vector<bool> *p_v)
    : CandidTypeVecBase() {}

CandidTypeVecBool::CandidTypeVecBool(const std::vector<bool> v)
    : CandidTypeVecBase() {}

CandidTypeVecBool::~CandidTypeVecBool() {}

void CandidTypeVecBool::set_content_type() {}

void CandidTypeVecBool::encode_M() {}

bool CandidTypeVecBool::decode_M(VecBytes B, __uint128_t &offset,
                                 std::string &parse_error) {
  return false;
}