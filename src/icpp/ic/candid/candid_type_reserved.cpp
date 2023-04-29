// The class for the Primitive Candid Type: reserved

#include "candid.h"
#include "ic_api.h"

#include "candid_opcode.h"
#include "pro.h"

CandidTypeReserved::CandidTypeReserved() : CandidTypePrim() {
  Pro().exit_if_not_pro();
}

CandidTypeReserved::~CandidTypeReserved() {}

void CandidTypeReserved::set_datatype() {}

void CandidTypeReserved::encode_I() {}

void CandidTypeReserved::encode_M() {}

bool CandidTypeReserved::decode_M(VecBytes B, __uint128_t &offset,
                                  std::string &parse_error) {
  return false;
}