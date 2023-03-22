// https://github.com/dfinity/candid/blob/master/spec/Candid.md

#pragma once

#include <variant>

// ------------------------------------------------------------------
// Forward declarations

// The helper classes
class CandidAssert;
class Opcode;
class VecBytes;

// The Candid Type base classes
class CandidTypeBase;
class CandidTypePrim;

// The Candid Type classes
// https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
// <primtype>
class CandidTypeNull;
class CandidTypeEmpty;
class CandidTypeReserved;
class CandidTypeBool;
class CandidTypeFloat32;
class CandidTypeFloat64;
class CandidTypeInt;
class CandidTypeNat;
class CandidTypeNat8;
class CandidTypeNat16;
class CandidTypeNat32;
class CandidTypeNat64;
class CandidTypeInt8;
class CandidTypeInt16;
class CandidTypeInt32;
class CandidTypeInt64;
class CandidTypeText;
class CandidTypePrincipal;

// <constype>
// class CandidTypeOpt;
// class CandidTypeVec;
class CandidTypeRecord;
// class CandidTypeVariant;

// The Candid Type Table class
class CandidTypeTable;

// The (De)Serialization classes
class CandidDeserialize;
class CandidSerialize;

// The std::variant alias
using CandidType =
    std::variant<CandidTypeNull, CandidTypeEmpty, CandidTypeReserved,
                 CandidTypeBool, CandidTypeFloat32, CandidTypeFloat64,
                 CandidTypeInt, CandidTypeNat, CandidTypeNat8, CandidTypeNat16,
                 CandidTypeNat32, CandidTypeNat64, CandidTypeInt8,
                 CandidTypeInt16, CandidTypeInt32, CandidTypeInt64,
                 CandidTypeText, CandidTypePrincipal, CandidTypeRecord>;

// https://subscription.packtpub.com/book/programming/9781787120495/8/ch08lvl1sec91/storing-different-types-with-stdvariant
template <typename T> bool is_CandidType(const CandidType &c) {
  return holds_alternative<T>(c);
}

// ------------------------------------------------------------------
// The include files

// The helper classes
#include "candid_assert.h"
#include "candid_opcode.h"
#include "vec_bytes.h"

// The Candid Type base classes
#include "candid_type_base.h"
#include "candid_type_prim.h"

// The Candid Type classes
// <primtype>
#include "candid_type_bool.h"
#include "candid_type_empty.h"
#include "candid_type_float32.h"
#include "candid_type_float64.h"
#include "candid_type_int.h"
#include "candid_type_int16.h"
#include "candid_type_int32.h"
#include "candid_type_int64.h"
#include "candid_type_int8.h"
#include "candid_type_nat.h"
#include "candid_type_nat16.h"
#include "candid_type_nat32.h"
#include "candid_type_nat64.h"
#include "candid_type_nat8.h"
#include "candid_type_null.h"
#include "candid_type_principal.h"
#include "candid_type_reserved.h"
#include "candid_type_text.h"

// <constype>
#include "candid_type_record.h"

// The Candid Type Table class
#include "candid_type_table.h"

// The (De)Serialization classes
#include "candid_deserialize.h"
#include "candid_serialize.h"