// The Candid type Opcodes and helper methods
//
//     https://github.com/dfinity/candid/blob/master/spec/Candid.md#types

#pragma once

#include <string>

#include "candid.h"
#include "ic_api.h"

class CandidOpcode {
public:
  CandidOpcode();
  ~CandidOpcode();

  bool is_primtype(const int &t);
  bool is_constype(const int &t);
  bool is_reftype(const int &t);
  void candid_type_from_opcode(CandidType &c, int opcode);

  // Primitive Types
  // T : <primtype> -> i8*
  int Null = -1;
  int Bool = -2;
  int Nat = -3;
  int Int = -4;
  int Nat8 = -5;
  int Nat16 = -6;
  int Nat32 = -7;
  int Nat64 = -8;
  int Int8 = -9;
  int Int16 = -10;
  int Int32 = -11;
  int Int64 = -12;
  int Float32 = -13;
  int Float64 = -14;
  int Text = -15;
  int Reserved = -16;
  int Empty = -17;
  int Principal = -24;

  // Constructed Types
  // T : <constype> -> i8*
  int Opt = -18;
  int Vec = -19;
  int Record = -20;
  int Variant = -21;

  // Reference Types
  // T : <reftype> -> i8*
  int Func = -22;
  int Service = -23;
};

class OpcodeHex {
public:
  // Primitive Types
  // T : <primtype> -> i8*
  uint8_t Null = 0x7f;
  uint8_t Bool = 0x7e;
  uint8_t Nat = 0x7d;
  uint8_t Int = 0x7c;
  uint8_t Nat8 = 0x7b;
  uint8_t Nat16 = 0x7a;
  uint8_t Nat32 = 0x79;
  uint8_t Nat64 = 0x78;
  uint8_t Int8 = 0x77;
  uint8_t Int16 = 0x76;
  uint8_t Int32 = 0x75;
  uint8_t Int64 = 0x74;
  uint8_t Float32 = 0x73;
  uint8_t Float64 = 0x72;
  uint8_t Text = 0x71;
  uint8_t Reserved = 0x70;
  uint8_t Empty = 0x6f;
  uint8_t Principal = 0x68;

  // Constructed Types
  // T : <constype> -> i8*
  uint8_t Opt = 0x6e;
  uint8_t Vec = 0x6d;
  uint8_t Record = 0x6c;
  uint8_t Variant = 0x6b;

  // Reference Types
  // T : <reftype> -> i8*
  uint8_t Func = 0x6a;
  uint8_t Service = 0x69;
};

class OpcodeTextual {
public:
  // Primitive Types
  // T : <primtype> -> i8*
  std::string Null = "null";
  std::string Bool = "bool";
  std::string Nat = "nat";
  std::string Int = "int";
  std::string Nat8 = "nat8";
  std::string Nat16 = "nat16";
  std::string Nat32 = "nat32";
  std::string Nat64 = "nat64";
  std::string Int8 = "int8";
  std::string Int16 = "int16";
  std::string Int32 = "int32";
  std::string Int64 = "int64";
  std::string Float32 = "float32";
  std::string Float64 = "float64";
  std::string Text = "text";
  std::string Reserved = "reserved";
  std::string Empty = "empty";
  std::string Principal = "principal";

  // Constructed Types
  // T : <constype> -> i8*
  std::string Opt = "opt";
  std::string Vec = "vec";
  std::string Record = "record";
  std::string Variant = "variant";

  // Reference Types
  // T : <reftype> -> i8*
  std::string Func = "func";
  std::string Service = "service";
};