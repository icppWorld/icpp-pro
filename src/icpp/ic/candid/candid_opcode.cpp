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
  if (opcode == Null) {
    c = CandidTypeNull();
  } else if (opcode == Empty) {
    c = CandidTypeEmpty();
  } else if (opcode == Reserved) {
    c = CandidTypeReserved();
  } else if (opcode == Bool) {
    c = CandidTypeBool();
  } else if (opcode == Float32) {
    c = CandidTypeFloat32();
  } else if (opcode == Float64) {
    c = CandidTypeFloat64();
  } else if (opcode == Int) {
    c = CandidTypeInt();
  } else if (opcode == Nat) {
    c = CandidTypeNat();
  } else if (opcode == Nat8) {
    c = CandidTypeNat8();
  } else if (opcode == Nat16) {
    c = CandidTypeNat16();
  } else if (opcode == Nat32) {
    c = CandidTypeNat32();
  } else if (opcode == Nat64) {
    c = CandidTypeNat64();
  } else if (opcode == Int8) {
    c = CandidTypeInt8();
  } else if (opcode == Int16) {
    c = CandidTypeInt16();
  } else if (opcode == Int32) {
    c = CandidTypeInt32();
  } else if (opcode == Int64) {
    c = CandidTypeInt64();
  } else if (opcode == Text) {
    c = CandidTypeText();
  } else if (opcode == Principal) {
    c = CandidTypePrincipal();
  } else if (opcode == Vec) {
    std::string msg;
    msg.append(
        "ERROR: NOT SUPPORTED FOR VEC. (use candid_vec_type_from_opcode)");
    msg.append("       datatype = " + std::to_string(opcode));
    msg.append("      " + std::string(__func__));
    IC_API::trap(msg);
  } else if (opcode == Record) {
    c = CandidTypeRecord();
  } else {
    std::string msg;
    msg.append("ERROR: NOT YET IMPLEMENTED FOR THIS OPCODE.");
    msg.append("       datatype = " + std::to_string(opcode));
    msg.append("      " + std::string(__func__));
    IC_API::trap(msg);
  }
}

void CandidOpcode::candid_type_vec_from_opcode(CandidType &c, int opcode) {
  // if (opcode == Null) {
  //   c = CandidTypeVecNull();
  // } else if (opcode == Empty) {
  //   c = CandidTypeVecEmpty();
  // } else if (opcode == Reserved) {
  //   c = CandidTypeVecReserved();
  // } else
  if (opcode == Bool) {
    c = CandidTypeVecBool();
  } else if (opcode == Float32) {
    c = CandidTypeVecFloat32();
  } else if (opcode == Float64) {
    c = CandidTypeVecFloat64();
  } else if (opcode == Int) {
    c = CandidTypeVecInt();
  } else if (opcode == Nat) {
    c = CandidTypeVecNat();
  } else if (opcode == Nat8) {
    c = CandidTypeVecNat8();
  } else if (opcode == Nat16) {
    c = CandidTypeVecNat16();
  } else if (opcode == Nat32) {
    c = CandidTypeVecNat32();
  } else if (opcode == Nat64) {
    c = CandidTypeVecNat64();
  } else if (opcode == Int8) {
    c = CandidTypeVecInt8();
  } else if (opcode == Int16) {
    c = CandidTypeVecInt16();
  } else if (opcode == Int32) {
    c = CandidTypeVecInt32();
  } else if (opcode == Int64) {
    c = CandidTypeVecInt64();
  } else if (opcode == Text) {
    c = CandidTypeVecText();
  } else if (opcode == Principal) {
    c = CandidTypeVecPrincipal();
    // } else if (opcode == Vec) {
    //   c = CandidTypeVecVec();
    // } else if (opcode == Record) {
    //   c = CandidTypeVecRecord();
  } else {
    std::string msg;
    msg.append("ERROR: NOT YET IMPLEMENTED CandidTypeVecXXX.");
    msg.append("       for content type = " + std::to_string(opcode));
    msg.append("      " + std::string(__func__));
    IC_API::trap(msg);
  }
}