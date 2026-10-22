// Tests for CandidTypeVecOf: a vec of any element type

#pragma once

#include <optional>
#include <string>
#include <vector>

#include "ic_api.h"

// record { id: nat64; note: opt text; kind: variant { a; b: nat };
//          tags: vec text; inner: record { q: text } }
struct VecOfItem {
  uint64_t id{0};
  std::optional<std::string> note;
  std::string kind;
  __uint128_t amount{0};
  std::vector<std::string> tags;
  std::string q;
};

// The type, with pointers into item, to decode into
CandidType vec_of_item_type(VecOfItem &item);
// The value, to encode
CandidType vec_of_item_value(const VecOfItem &item);
