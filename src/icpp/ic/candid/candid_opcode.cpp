// The Candid type Opcodes and helper methods

#include <string>

#include "candid.h"
#include "ic_api.h"

CandidOpcode::CandidOpcode() {}

CandidOpcode::~CandidOpcode() {}

bool CandidOpcode::is_primtype(const int &t) {
  if (t == -1 || t == -2 || t == -3 || t == -4 || t == -5 || t == -6 ||
      t == -7 || t == -8 || t == -9 || t == -10 || t == -11 || t == -12 ||
      t == -13 || t == -14 || t == -15 || t == -16 || t == -17 || t == -24)
    return true;
  return false;
}

bool CandidOpcode::is_constype(const int &t) {
  if (t == -18 || t == -19 || t == -20 || t == -21) return true;
  return false;
}

bool CandidOpcode::is_reftype(const int &t) {
  if (t == -22 || t == -23) return true;
  return false;
}

void CandidOpcode::candid_type_from_opcode(CandidType &c, int opcode) {
  if (opcode == Bool) {
    c = CandidTypeBool();
  } else if (opcode == Empty) {
    c = CandidTypeEmpty();
  } else if (opcode == Float64) {
    c = CandidTypeFloat64();
  } else if (opcode == Int) {
    c = CandidTypeInt();
  } else if (opcode == Nat) {
    c = CandidTypeNat();
  } else if (opcode == Record) {
    c = CandidTypeRecord();
  } else if (opcode == Text) {
    c = CandidTypeText();
  } else {
    std::string msg;
    msg.append("ERROR: NOT YET IMPLEMENTED FOR THIS OPCODE.");
    msg.append("       datatype = " + std::to_string(opcode));
    msg.append("      " + std::string(__func__));
    IC_API::trap(msg);
  }
}