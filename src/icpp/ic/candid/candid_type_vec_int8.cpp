// The class for the Candid Type: vec

#include "candid.h"
#include "candid_opcode.h"

#include <cassert>

#include "ic_api.h"

CandidTypeVecInt8::CandidTypeVecInt8() : CandidTypeVecBase() {}

CandidTypeVecInt8::CandidTypeVecInt8(std::vector<int8_t> *p_v)
    : CandidTypeVecBase() {}

CandidTypeVecInt8::CandidTypeVecInt8(const std::vector<int8_t> v)
    : CandidTypeVecBase() {}

CandidTypeVecInt8::~CandidTypeVecInt8() {}

void CandidTypeVecInt8::set_content_type() {}

void CandidTypeVecInt8::encode_M() {}

bool CandidTypeVecInt8::decode_M(VecBytes B, __uint128_t &offset,
                                 std::string &parse_error) {
  return false;
}