// The class for the Candid Type: opt

#include "candid.h"
#include "candid_opcode.h"

#include <cassert>

#include "ic_api.h"

CandidTypeOptInt16::CandidTypeOptInt16() : CandidTypeOptBase() {}

CandidTypeOptInt16::CandidTypeOptInt16(std::optional<int16_t> *p_v)
    : CandidTypeOptBase() {}

CandidTypeOptInt16::CandidTypeOptInt16(const std::optional<int16_t> v)
    : CandidTypeOptBase() {}

CandidTypeOptInt16::~CandidTypeOptInt16() {}

void CandidTypeOptInt16::set_content_type() {}

void CandidTypeOptInt16::encode_M() {}

bool CandidTypeOptInt16::decode_M(VecBytes B, __uint128_t &offset,
                                  std::string &parse_error) {
  return false;
}