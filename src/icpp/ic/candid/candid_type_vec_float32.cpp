// The class for the Candid Type: vec

#include "candid.h"
#include "candid_opcode.h"

#include <cassert>

#include "ic_api.h"

CandidTypeVecFloat32::CandidTypeVecFloat32() : CandidTypeVecBase() {}

CandidTypeVecFloat32::CandidTypeVecFloat32(std::vector<float> *p_v)
    : CandidTypeVecBase() {}

CandidTypeVecFloat32::CandidTypeVecFloat32(const std::vector<float> v)
    : CandidTypeVecBase() {}

CandidTypeVecFloat32::~CandidTypeVecFloat32() {}

void CandidTypeVecFloat32::set_content_type() {}

void CandidTypeVecFloat32::encode_M() {}

bool CandidTypeVecFloat32::decode_M(VecBytes B, __uint128_t &offset,
                                    std::string &parse_error) {
  return false;
}