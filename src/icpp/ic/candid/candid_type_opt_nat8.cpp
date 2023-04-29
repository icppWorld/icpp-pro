// The class for the Candid Type: opt

#include "candid.h"
#include "candid_opcode.h"

#include <cassert>

#include "ic_api.h"

CandidTypeOptNat8::CandidTypeOptNat8() : CandidTypeOptBase() {}

CandidTypeOptNat8::CandidTypeOptNat8(std::optional<uint8_t> *p_v)
    : CandidTypeOptBase() {}

CandidTypeOptNat8::CandidTypeOptNat8(const std::optional<uint8_t> v)
    : CandidTypeOptBase() {}

CandidTypeOptNat8::~CandidTypeOptNat8() {}

void CandidTypeOptNat8::set_content_type() {}

void CandidTypeOptNat8::encode_M() {}

bool CandidTypeOptNat8::decode_M(VecBytes B, __uint128_t &offset,
                                 std::string &parse_error) {
  return false;
}