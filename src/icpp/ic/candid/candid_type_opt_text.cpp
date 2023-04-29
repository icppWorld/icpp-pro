// The class for the Candid Type: opt

#include "candid.h"
#include "candid_opcode.h"

#include <cassert>

#include "ic_api.h"

CandidTypeOptText::CandidTypeOptText() : CandidTypeOptBase() {}

CandidTypeOptText::CandidTypeOptText(std::optional<std::string> *p_v)
    : CandidTypeOptBase() {}

// These constructors are only for encoding
CandidTypeOptText::CandidTypeOptText(const std::optional<std::string> v)
    : CandidTypeOptBase() {}

CandidTypeOptText::~CandidTypeOptText() {}

void CandidTypeOptText::set_content_type() {}

void CandidTypeOptText::encode_M() {}

// Decode the values, starting at & updating offset
bool CandidTypeOptText::decode_M(VecBytes B, __uint128_t &offset,
                                 std::string &parse_error) {
  return false;
}