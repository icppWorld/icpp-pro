// The class for the Candid Type: opt

#include "candid.h"
#include "candid_opcode.h"

#include <cassert>

#include "ic_api.h"

CandidTypeOptInt8::CandidTypeOptInt8() : CandidTypeOptBase() {}

CandidTypeOptInt8::CandidTypeOptInt8(std::optional<int8_t> *p_v)
    : CandidTypeOptBase() {}

CandidTypeOptInt8::CandidTypeOptInt8(const std::optional<int8_t> v)
    : CandidTypeOptBase() {}

CandidTypeOptInt8::~CandidTypeOptInt8() {}

void CandidTypeOptInt8::set_content_type() {}

void CandidTypeOptInt8::encode_M() {}

bool CandidTypeOptInt8::decode_M(VecBytes B, __uint128_t &offset,
                                 std::string &parse_error) {
  return false;
}