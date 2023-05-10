// https://github.com/dfinity/candid/blob/master/spec/Candid.md

#pragma once

#include <optional>
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
class CandidTypeVecBase;
class CandidTypeOptBase;

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
class CandidTypeVecBool;
class CandidTypeVecFloat32;
class CandidTypeVecFloat64;
class CandidTypeVecInt;
class CandidTypeVecNat;
class CandidTypeVecNat8;
class CandidTypeVecNat16;
class CandidTypeVecNat32;
class CandidTypeVecNat64;
class CandidTypeVecInt8;
class CandidTypeVecInt16;
class CandidTypeVecInt32;
class CandidTypeVecInt64;
class CandidTypeVecText;
class CandidTypeVecPrincipal;
// These types are not used for vecs
// class CandidTypeVecNull;
// class CandidTypeVecEmpty;
// class CandidTypeVecReserved;

class CandidTypeOptBool;
class CandidTypeOptFloat32;
class CandidTypeOptFloat64;
class CandidTypeOptInt;
class CandidTypeOptNat;
class CandidTypeOptNat8;
class CandidTypeOptNat16;
class CandidTypeOptNat32;
class CandidTypeOptNat64;
class CandidTypeOptInt8;
class CandidTypeOptInt16;
class CandidTypeOptInt32;
class CandidTypeOptInt64;
class CandidTypeOptText;
class CandidTypeOptPrincipal;
// class CandidTypeOptVec;
// class CandidTypeOptRecord;
// These types are not used for opts
// class CandidTypeOptNull;
// class CandidTypeOptEmpty;
// class CandidTypeOptReserved;

class CandidTypeRecord;
class CandidTypeVariant;

// The Candid Type Table class
class CandidTypeTable;

// The (De)Serialization classes
class CandidDeserialize;
class CandidSerialize;

// The std::variant alias
using CandidType = std::variant<
    CandidTypeNull, CandidTypeEmpty, CandidTypeReserved, CandidTypeBool,
    CandidTypeFloat32, CandidTypeFloat64, CandidTypeInt, CandidTypeNat,
    CandidTypeNat8, CandidTypeNat16, CandidTypeNat32, CandidTypeNat64,
    CandidTypeInt8, CandidTypeInt16, CandidTypeInt32, CandidTypeInt64,
    CandidTypeText, CandidTypePrincipal, CandidTypeVecBool,
    CandidTypeVecFloat32, CandidTypeVecFloat64, CandidTypeVecInt,
    CandidTypeVecNat, CandidTypeVecNat8, CandidTypeVecNat16, CandidTypeVecNat32,
    CandidTypeVecNat64, CandidTypeVecInt8, CandidTypeVecInt16,
    CandidTypeVecInt32, CandidTypeVecInt64, CandidTypeVecText,
    CandidTypeVecPrincipal, CandidTypeOptBool, CandidTypeOptFloat32,
    CandidTypeOptFloat64, CandidTypeOptInt, CandidTypeOptNat, CandidTypeOptNat8,
    CandidTypeOptNat16, CandidTypeOptNat32, CandidTypeOptNat64,
    CandidTypeOptInt8, CandidTypeOptInt16, CandidTypeOptInt32,
    CandidTypeOptInt64, CandidTypeOptText, CandidTypeOptPrincipal,
    CandidTypeRecord, CandidTypeVariant>;

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
#include "candid_type_opt_base.h"
#include "candid_type_prim.h"
#include "candid_type_vec_base.h"

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
#include "candid_type_vec_bool.h"
#include "candid_type_vec_float32.h"
#include "candid_type_vec_float64.h"
#include "candid_type_vec_int.h"
#include "candid_type_vec_int16.h"
#include "candid_type_vec_int32.h"
#include "candid_type_vec_int64.h"
#include "candid_type_vec_int8.h"
#include "candid_type_vec_nat.h"
#include "candid_type_vec_nat16.h"
#include "candid_type_vec_nat32.h"
#include "candid_type_vec_nat64.h"
#include "candid_type_vec_nat8.h"
#include "candid_type_vec_principal.h"
#include "candid_type_vec_text.h"
// These types are not used for vectors
// #include "candid_type_vec_empty.h"
// #include "candid_type_vec_null.h"
// #include "candid_type_vec_reserved.h"

#include "candid_type_opt_bool.h"
#include "candid_type_opt_float32.h"
#include "candid_type_opt_float64.h"
#include "candid_type_opt_int.h"
#include "candid_type_opt_int16.h"
#include "candid_type_opt_int32.h"
#include "candid_type_opt_int64.h"
#include "candid_type_opt_int8.h"
#include "candid_type_opt_nat.h"
#include "candid_type_opt_nat16.h"
#include "candid_type_opt_nat32.h"
#include "candid_type_opt_nat64.h"
#include "candid_type_opt_nat8.h"
#include "candid_type_opt_principal.h"
#include "candid_type_opt_text.h"
// #include "candid_type_opt_vec.h"
// #include "candid_type_opt_record.h"
// These types are not used for opts
// #include "candid_type_opt_empty.h"
// #include "candid_type_opt_null.h"
// #include "candid_type_opt_reserved.h"

#include "candid_type_record.h"
#include "candid_type_variant.h"

// The Candid Type Table class
#include "candid_type_table.h"

// The (De)Serialization classes
#include "candid_deserialize.h"
#include "candid_serialize.h"