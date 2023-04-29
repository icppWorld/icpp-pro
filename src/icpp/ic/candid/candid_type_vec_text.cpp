// The class for the Candid Type: vec

#include "candid.h"
#include "candid_opcode.h"

#include <cassert>

#include "ic_api.h"

CandidTypeVecText::CandidTypeVecText() : CandidTypeVecBase() {}

CandidTypeVecText::CandidTypeVecText(std::vector<std::string> *p_v)
    : CandidTypeVecBase() {}

CandidTypeVecText::CandidTypeVecText(const std::vector<std::string> v)
    : CandidTypeVecBase() {}

CandidTypeVecText::~CandidTypeVecText() {}

void CandidTypeVecText::set_content_type() {}

void CandidTypeVecText::encode_M() {}

bool CandidTypeVecText::decode_M(VecBytes B, __uint128_t &offset,
                                 std::string &parse_error) {
  return false;
}