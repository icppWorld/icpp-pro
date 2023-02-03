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
class CandidTypeBool;
class CandidTypeEmpty;
class CandidTypeFloat64;
class CandidTypeInt;
class CandidTypeNat;
class CandidTypeNull;
class CandidTypeRecord;
class CandidTypeText;

// The Candid Type Table class
class CandidTypeTable;

// The (De)Serialization classes
class CandidDeserialize;
class CandidSerialize;

// The std::variant alias
using CandidType =
    std::variant<CandidTypeBool, CandidTypeEmpty, CandidTypeFloat64,
                 CandidTypeInt, CandidTypeNat, CandidTypeNull, CandidTypeRecord,
                 CandidTypeText>;

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
#include "candid_type_bool.h"
#include "candid_type_empty.h"
#include "candid_type_float64.h"
#include "candid_type_int.h"
#include "candid_type_nat.h"
#include "candid_type_null.h"
#include "candid_type_record.h"
#include "candid_type_text.h"

// The Candid Type Table class
#include "candid_type_table.h"

// The (De)Serialization classes
#include "candid_deserialize.h"
#include "candid_serialize.h"