// The class for the Candid Type: opt

#include "candid.h"
#include "candid_opcode.h"

#include <cassert>

#include "ic_api.h"

CandidTypeOptFloat32::CandidTypeOptFloat32() : CandidTypeOptBase() {
  std::optional<float> v;
}

CandidTypeOptFloat32::CandidTypeOptFloat32(std::optional<float> *p_v)
    : CandidTypeOptBase() {}

CandidTypeOptFloat32::CandidTypeOptFloat32(const std::optional<float> v)
    : CandidTypeOptBase() {}

CandidTypeOptFloat32::~CandidTypeOptFloat32() {}

void CandidTypeOptFloat32::set_content_type() {}

void CandidTypeOptFloat32::encode_M() {}

// Decode the values, starting at & updating offset
bool CandidTypeOptFloat32::decode_M(VecBytes B, __uint128_t &offset,
                                    std::string &parse_error) {

  return false;
}