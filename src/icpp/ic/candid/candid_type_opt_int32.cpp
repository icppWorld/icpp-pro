// The class for the Candid Type: opt

#include "candid.h"
#include "candid_opcode.h"

#include <cassert>

#include "ic_api.h"

CandidTypeOptInt32::CandidTypeOptInt32() : CandidTypeOptBase() {}

CandidTypeOptInt32::CandidTypeOptInt32(std::optional<int32_t> *p_v)
    : CandidTypeOptBase() {}

CandidTypeOptInt32::CandidTypeOptInt32(const std::optional<int32_t> v)
    : CandidTypeOptBase() {}

CandidTypeOptInt32::~CandidTypeOptInt32() {}

void CandidTypeOptInt32::set_content_type() {}

void CandidTypeOptInt32::encode_M() {}

// Decode the values, starting at & updating offset
bool CandidTypeOptInt32::decode_M(VecBytes B, __uint128_t &offset,
                                  std::string &parse_error) {
  return false;
}