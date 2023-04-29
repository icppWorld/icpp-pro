// The class for the Candid Type: vec

#include "candid.h"
#include "candid_opcode.h"

#include <cassert>

#include "ic_api.h"

CandidTypeVecPrincipal::CandidTypeVecPrincipal() : CandidTypeVecBase() {}

CandidTypeVecPrincipal::CandidTypeVecPrincipal(std::vector<std::string> *p_v)
    : CandidTypeVecBase() {}

CandidTypeVecPrincipal::CandidTypeVecPrincipal(const std::vector<std::string> v)
    : CandidTypeVecBase() {}

CandidTypeVecPrincipal::~CandidTypeVecPrincipal() {}

void CandidTypeVecPrincipal::set_content_type() {}

void CandidTypeVecPrincipal::encode_M() {}

bool CandidTypeVecPrincipal::decode_M(VecBytes B, __uint128_t &offset,
                                      std::string &parse_error) {
  return false;
}