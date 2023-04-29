// The class for the Candid Type: vec

#include "candid.h"
#include "candid_opcode.h"

#include <cassert>

#include "ic_api.h"

CandidTypeVecInt16::CandidTypeVecInt16() : CandidTypeVecBase() {}

CandidTypeVecInt16::CandidTypeVecInt16(std::vector<int16_t> *p_v)
    : CandidTypeVecBase() {}

CandidTypeVecInt16::CandidTypeVecInt16(const std::vector<int16_t> v)
    : CandidTypeVecBase() {}

CandidTypeVecInt16::~CandidTypeVecInt16() {}

void CandidTypeVecInt16::set_content_type() {}

void CandidTypeVecInt16::encode_M() {}

bool CandidTypeVecInt16::decode_M(VecBytes B, __uint128_t &offset,
                                  std::string &parse_error) {
  return false;
}