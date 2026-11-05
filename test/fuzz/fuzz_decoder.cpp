// libFuzzer target: CandidDecoder, the decoder every canister uses.
//
// The first byte of the input picks the expected types, the rest is the
// message. The memory the decoder holds is checked against the size of the
// message (memory_oracle.h). A message that decodes is also printed as text
// and parsed back with CandidTextValue at the same types, when its values,
// written out in full, are not many more than its bytes.
//
// Build and run with `make fuzz` from the root of icpp-pro.

#include "candid_decoder.h"
#include "candid_text_value.h"
#include "candid_type_graph.h"
#include "candid_value.h"
#include "memory_oracle.h"

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace {

int prim(CandidKind k) { return CandidTypeGraph::prim(k); }

struct Expected {
  CandidTypeGraph g;
  std::vector<int> args;
};

// The expected types, one per value of the first byte (mod the count)
std::vector<Expected> make_expected() {
  std::vector<Expected> out;
  int nat = prim(CandidKind::Nat), text = prim(CandidKind::Text);

  { // Skip everything
    Expected e;
    int r = prim(CandidKind::Reserved);
    e.args = {r, r, r};
    out.push_back(e);
  }
  { // Primitives, and coercions to them
    Expected e;
    e.args = {nat,
              prim(CandidKind::Int),
              e.g.opt(nat),
              text,
              e.g.vec(prim(CandidKind::Nat8)),
              prim(CandidKind::Principal),
              prim(CandidKind::Float64),
              prim(CandidKind::Bool)};
    out.push_back(e);
  }
  { // Records, variants and vecs
    Expected e;
    int v =
        e.g.variant({{0, prim(CandidKind::Null)}, {1, prim(CandidKind::Int)}});
    int r = e.g.record({{97, nat}, {98, e.g.opt(text)}, {99, e.g.vec(v)}});
    e.args = {r, e.g.vec(r), e.g.opt(e.g.record({}))};
    out.push_back(e);
  }
  { // A recursive type: type List = opt record { head : int; tail : List }
    Expected e;
    int list = e.g.reserve();
    int node = e.g.record({{0, prim(CandidKind::Int)}, {1, list}});
    CandidGraphNode n;
    n.kind = CandidKind::Opt;
    n.inner = node;
    e.g.define(list, n);
    e.args = {list, list};
    out.push_back(e);
  }
  { // References: func and service
    Expected e;
    int f = e.g.func({text}, {nat}, {CANDID_FUNC_QUERY});
    int m = e.g.func({nat}, {}, {});
    int s = e.g.service({{"get", f}, {"set", m}});
    e.args = {f, s, e.g.opt(f)};
    out.push_back(e);
  }
  { // Nested opts, and types with no values
    Expected e;
    e.args = {e.g.opt(e.g.opt(e.g.opt(nat))), e.g.vec(e.g.opt(e.g.record({}))),
              prim(CandidKind::Null), e.g.opt(prim(CandidKind::Empty))};
    out.push_back(e);
  }
  { // Fixed-width numbers in a variant
    Expected e;
    e.args = {e.g.variant({{0, prim(CandidKind::Float64)},
                           {1, prim(CandidKind::Float32)},
                           {2, prim(CandidKind::Int64)},
                           {3, prim(CandidKind::Nat32)},
                           {4, prim(CandidKind::Int8)}})};
    out.push_back(e);
  }
  return out;
}

// The number of values in v, written out in full (shared records once per
// place they are in), or more than `cap`
size_t expanded(const CandidValue &v, size_t cap,
                std::map<const CandidValue *, size_t> &memo) {
  size_t n = 1;
  auto add = [&](const CandidValue &c) {
    if (n <= cap) n += expanded(c, cap, memo);
  };
  if (v.kind == CandidKind::Record) {
    for (size_t k = 0; k < v.items.size(); ++k)
      add(v.items[k]);
    if (v.rest) {
      const CandidValue *r = v.rest.get();
      auto it = memo.find(r);
      if (it == memo.end()) {
        size_t m = 0;
        for (const auto &c : r->items)
          if (m <= cap) m += expanded(c, cap, memo);
        it = memo.emplace(r, m).first;
      }
      n += it->second;
    }
  } else if (v.repeat) {
    size_t one = expanded(v.items[0], cap, memo);
    n += one > cap / v.repeat ? cap + 1 : one * v.repeat;
  } else {
    for (const auto &c : v.items)
      add(c);
  }
  return std::min(n, cap + 1);
}

void decode_and_reparse(const uint8_t *data, size_t size,
                        const CandidTypeGraph &g,
                        const std::vector<int> &args) {
  std::vector<CandidValue> values;
  std::string err;
  MemoryOracle::start();
  bool ok = CandidDecoder::decode(data, size, g, args, values, err);
  MemoryOracle::check(size, "CandidDecoder::decode");
  if (!ok) return;

  size_t cap = 16 * size + 1024, n = 0;
  std::map<const CandidValue *, size_t> memo;
  for (const auto &v : values)
    if (n <= cap) n += expanded(v, cap, memo);
  if (n > cap) return;
  std::string text = candid_values_to_text(values);
  std::vector<CandidValue> reparsed;
  CandidTextValue::parse(text, g, args, reparsed, err);
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  static const std::vector<Expected> expected = make_expected();
  if (size < 1) return 0;
  size_t pick = data[0] % (expected.size() + 1);
  ++data;
  --size;

  if (pick == expected.size()) {
    // At the message's own types
    CandidTypeGraph wire;
    std::vector<int> args;
    std::string err;
    if (CandidDecoder::read_types(data, size, wire, args, err))
      decode_and_reparse(data, size, wire, args);
    return 0;
  }
  decode_and_reparse(data, size, expected[pick].g, expected[pick].args);
  return 0;
}
