// libFuzzer target: CandidDecoder, the decoder every canister uses.
//
// The first byte of the input picks the expected types (expected_types.h),
// the rest is the message. The memory the decoder holds is checked against
// the size of the message (memory_oracle.h). A message that decodes, when its
// values, written out in full, are not many more than its bytes, is also:
// - printed as text and parsed back with CandidTextValue at the same types;
// - encoded again with CandidEncoder at the same types. That must succeed,
//   decode to equal values, and encode again to the same bytes.
//
// Build and run with `make fuzz` from the root of icpp-pro.

#include "candid_decoder.h"
#include "candid_encoder.h"
#include "candid_text_value.h"
#include "candid_type_graph.h"
#include "candid_value.h"
#include "expected_types.h"
#include "memory_oracle.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <string>
#include <vector>

namespace {

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

[[noreturn]] void fail(const std::string &what) {
  std::fprintf(stderr, "fuzz_decoder: %s\n", what.c_str());
  std::abort();
}

// A type of a future version of Candid, which a message may have and the
// decoder skips, but the encoder cannot write
bool has_future(const CandidTypeGraph &g, const std::vector<int> &args) {
  auto future = [&](int t) { return g.kind(t) == CandidKind::Future; };
  for (int t : args)
    if (future(t)) return true;
  for (size_t i = 0; i < g.size(); ++i) {
    const CandidGraphNode &n = g.node(int(i));
    if (n.inner != -1 && future(n.inner)) return true;
    for (const auto &f : n.fields)
      if (future(f.type)) return true;
    for (int t : n.args)
      if (future(t)) return true;
    for (int t : n.rets)
      if (future(t)) return true;
  }
  return false;
}

// Encodes the decoded values again, at the types they were decoded at
void reencode(const CandidTypeGraph &g, const std::vector<int> &args,
              const std::vector<CandidValue> &values) {
  std::vector<uint8_t> once, twice;
  std::vector<CandidValue> again;
  std::string err;
  if (!CandidEncoder::encode(g, args, values, once, err))
    fail("a decoded value does not encode: " + err);
  if (!CandidDecoder::decode(once.data(), once.size(), g, args, again, err))
    fail("an encoded value does not decode: " + err);
  if (again != values) fail("an encoded value decodes to another value");
  if (!CandidEncoder::encode(g, args, again, twice, err) || twice != once)
    fail("a value encodes to other bytes the second time");
}

void decode_and_roundtrip(const uint8_t *data, size_t size,
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
  if (!has_future(g, args)) reencode(g, args, values);
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  static const std::vector<FuzzExpected> expected = make_fuzz_expected();
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
      decode_and_roundtrip(data, size, wire, args);
    return 0;
  }
  decode_and_roundtrip(data, size, expected[pick].g, expected[pick].args);
  return 0;
}
