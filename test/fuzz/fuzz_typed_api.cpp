// libFuzzer target: the typed API, CandidDeserialize into CandidType* objects
// bound to C++ variables, as a canister method reads its arguments.
//
// The first byte of the input picks the arguments, the rest is the message.
// A trap is a rejected message and is caught (see fuzz_ic0.cpp). The memory
// the decoder holds is checked against the size of the message
// (memory_oracle.h).
//
// Build and run with `make fuzz` from the root of icpp-pro.

#include "candid_args.h"
#include "candid_deserialize.h"
#include "candid_type.h"
#include "candid_type_all_includes.h"
#include "vec_bytes.h"

#include "ic0.h"
#include "memory_oracle.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace {

// The variables the arguments are bound to
struct Vars {
  bool b{false};
  __uint128_t n{0};
  __int128_t i{0};
  int64_t i64{0};
  double f64{0};
  std::string t, p, canister, method, label;
  std::optional<__uint128_t> on;
  std::optional<std::string> ot;
  std::vector<uint8_t> blob;
  std::vector<std::string> texts;
  std::vector<__uint128_t> col_n;
  std::vector<std::string> col_t;
  bool has{false};
  size_t count{0};
  // The columns of a vec record: CandidTypeVecRecord keeps a pointer to it
  CandidTypeRecord columns;
};

CandidArgs make_args(size_t pick, Vars &v) {
  CandidArgs A;
  switch (pick) {
  case 0: // Primitives
    A.append(CandidTypeBool{&v.b});
    A.append(CandidTypeNat{&v.n});
    A.append(CandidTypeInt{&v.i});
    A.append(CandidTypeInt64{&v.i64});
    A.append(CandidTypeFloat64{&v.f64});
    A.append(CandidTypeText{&v.t});
    A.append(CandidTypePrincipal{&v.p});
    break;
  case 1: { // A record with optional and vec fields
    CandidTypeRecord r;
    r.append("n", CandidTypeNat{&v.n});
    r.append("ot", CandidTypeOptText{&v.ot});
    r.append("blob", CandidTypeVecNat8{&v.blob});
    r.append_optional("on", CandidTypeOptNat{&v.on});
    A.append(r);
    break;
  }
  case 2: { // A variant, and a vec of variants
    CandidTypeVariant var{&v.label};
    var.append("a", CandidTypeNat{&v.n});
    var.append("b", CandidTypeText{&v.t});
    var.append("c", CandidTypeNull{});
    A.append(var);
    break;
  }
  case 3: // A vec of records, decoded column by column
    v.columns.append("n", CandidTypeVecNat{&v.col_n});
    v.columns.append("t", CandidTypeVecText{&v.col_t});
    A.append(CandidTypeVecRecord{&v.columns});
    break;
  case 4: { // A vec of any element type, and an opt of any type
    A.append(CandidTypeVecOf(
        [&]() -> CandidType {
          CandidTypeRecord r;
          r.append("t", CandidTypeText{&v.t});
          return r;
        },
        [&]() { ++v.count; }));
    A.append(CandidTypeOptOf(
        [&]() -> CandidType { return CandidTypeVecText{&v.texts}; }, &v.has));
    break;
  }
  default: { // References
    CandidTypeFunc f{&v.canister, &v.method};
    f.append_arg(CandidTypeText{});
    f.append_ret(CandidTypeNat{});
    A.append(f);
    CandidTypeService s{&v.p};
    s.append_method("get", f);
    A.append(s);
    break;
  }
  }
  return A;
}

constexpr size_t PICKS = 6;

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  if (size < 1 || size > UINT32_MAX) return 0;
  size_t pick = data[0] % PICKS;
  VecBytes B;
  B.append_bytes(data + 1, uint32_t(size - 1));

  Vars v;
  MemoryOracle::start();
  try {
    CandidArgs A = make_args(pick, v);
    CandidDeserialize(B, A);
  } catch (const FuzzTrap &) {
    // A rejected message
  }
  MemoryOracle::check(size, "CandidDeserialize");
  return 0;
}
