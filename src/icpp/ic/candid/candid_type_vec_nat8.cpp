// The class for the Candid Type: vec

#include "candid.h"
#include "candid_opcode.h"

#include <cassert>

#include "ic_api.h"

CandidTypeVecNat8::CandidTypeVecNat8() : CandidTypeVecBase() {}

CandidTypeVecNat8::CandidTypeVecNat8(std::vector<uint8_t> *p_v)
    : CandidTypeVecBase() {}

CandidTypeVecNat8::CandidTypeVecNat8(const std::vector<uint8_t> v)
    : CandidTypeVecBase() {}

CandidTypeVecNat8::~CandidTypeVecNat8() {}

void CandidTypeVecNat8::set_content_type() {}

void CandidTypeVecNat8::encode_M() {}

bool CandidTypeVecNat8::decode_M(VecBytes B, __uint128_t &offset,
                                 std::string &parse_error) {
  return false;
}