// libFuzzer target: CandidEncoder, which encodes the CandidValues a canister
// builds, CandidTypeValue's included.
//
// The first byte of the input picks the expected types (expected_types.h),
// the rest drives the building of one value per argument: mostly of the
// argument's type, with some parts of the wrong kind, out of range, with
// unknown ids, malformed vecs, bad UTF-8, long principals, and so on.
//
// The encoder must refuse such a value, or encode it to bytes that decode at
// the same types and encode again to the same bytes. The memory it holds is
// checked against the size of the input and the output (memory_oracle.h).
//
// Build and run with `make fuzz` from the root of icpp-pro.

#include "candid_decoder.h"
#include "candid_encoder.h"
#include "candid_type_graph.h"
#include "candid_value.h"
#include "expected_types.h"
#include "memory_oracle.h"

#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

namespace {

// The bytes of the input, read one at a time; 0 once they run out
class Reader {
public:
  Reader(const uint8_t *data, size_t size) : m_data(data), m_size(size) {}
  bool done() const { return m_pos >= m_size; }
  uint8_t byte() { return done() ? 0 : m_data[m_pos++]; }
  uint64_t u64() {
    uint64_t v = 0;
    for (int k = 0; k < 8; ++k)
      v = v << 8 | byte();
    return v;
  }
  std::string text(size_t max) {
    size_t n = byte() % (max + 1);
    std::string s;
    for (size_t k = 0; k < n && !done(); ++k)
      s.push_back(char(byte()));
    return s;
  }

private:
  const uint8_t *m_data;
  size_t m_size;
  size_t m_pos{0};
};

const CandidKind ALL_KINDS[] = {
    CandidKind::Null,     CandidKind::Bool,    CandidKind::Nat,
    CandidKind::Int,      CandidKind::Nat8,    CandidKind::Nat16,
    CandidKind::Nat32,    CandidKind::Nat64,   CandidKind::Int8,
    CandidKind::Int16,    CandidKind::Int32,   CandidKind::Int64,
    CandidKind::Float32,  CandidKind::Float64, CandidKind::Text,
    CandidKind::Reserved, CandidKind::Empty,   CandidKind::Principal,
    CandidKind::Opt,      CandidKind::Vec,     CandidKind::Record,
    CandidKind::Variant,  CandidKind::Func,    CandidKind::Service};

class Builder {
public:
  Builder(const CandidTypeGraph &g, Reader &r) : m_g(g), m_r(r) {}

  CandidValue value(int t, size_t depth) {
    CandidKind kind = m_g.kind(t);
    uint8_t mutate = m_r.byte();
    // Now and then a part of the wrong kind
    if (mutate < 8) kind = ALL_KINDS[m_r.byte() % std::size(ALL_KINDS)];
    CandidValue v;
    v.kind = kind;
    // Deep enough: no more children, but now and then past the depth limit
    bool stop = m_r.done() || (depth > 40 && mutate % 4 != 0) || depth > 200;
    switch (kind) {
    case CandidKind::Bool:
      v.b = m_r.byte() & 1;
      break;
    case CandidKind::Nat:
    case CandidKind::Int: {
      // Up to 256 bits, of either sign, also for a nat
      __uint128_t hi = m_r.u64(), lo = m_r.u64();
      v.n = CandidBigInt::from_u128(hi << 64 | lo);
      if (mutate & 0x10) v.n = CandidBigInt::from_i128(-__int128_t(lo));
      if (mutate & 0x20) {
        std::string digits = v.n.to_string() + v.n.to_string();
        CandidBigInt::parse(digits, v.n);
      }
      break;
    }
    case CandidKind::Nat8:
    case CandidKind::Nat16:
    case CandidKind::Nat32:
    case CandidKind::Nat64:
      v.u = m_r.u64() >> (m_r.byte() % 64);
      break;
    case CandidKind::Int8:
    case CandidKind::Int16:
    case CandidKind::Int32:
    case CandidKind::Int64:
      v.i = int64_t(m_r.u64()) >> (m_r.byte() % 64);
      break;
    case CandidKind::Float32:
    case CandidKind::Float64: {
      uint64_t bits = m_r.u64();
      std::memcpy(&v.f64, &bits, sizeof(bits));
      break;
    }
    case CandidKind::Text:
      v.text = m_r.text(40); // any bytes: not always UTF-8
      break;
    case CandidKind::Principal:
    case CandidKind::Service:
      v.text.clear();
      for (size_t k = 0, n = m_r.byte() % 34; k < n; ++k)
        v.bytes.push_back(m_r.byte());
      break;
    case CandidKind::Func:
      for (size_t k = 0, n = m_r.byte() % 34; k < n; ++k)
        v.bytes.push_back(m_r.byte());
      v.text = m_r.text(20);
      break;
    case CandidKind::Opt:
      if (!stop && (m_r.byte() & 1)) v.items.push_back(child(t, depth));
      if (mutate == 255 && !stop) v.items.push_back(child(t, depth));
      break;
    case CandidKind::Vec:
      vec(t, v, mutate, stop, depth);
      break;
    case CandidKind::Record:
      record(t, v, mutate, stop, depth);
      break;
    case CandidKind::Variant: {
      const auto *fields = node_fields(t, CandidKind::Variant);
      uint8_t pick = m_r.byte();
      uint32_t id = fields && !fields->empty() && pick < 240
                        ? (*fields)[pick % fields->size()].id
                        : uint32_t(m_r.u64());
      v.ids = {id};
      int ct = field_type(fields, id);
      v.items = {stop || ct == INT32_MIN ? CandidValue{}
                                         : value(ct, depth + 1)};
      break;
    }
    default: // null, reserved, empty, future
      break;
    }
    if (mutate == 254) v.absent = true;
    return v;
  }

private:
  const CandidTypeGraph &m_g;
  Reader &m_r;

  const std::vector<CandidGraphField> *node_fields(int t, CandidKind k) {
    if (!CandidTypeGraph::is_node(t) || m_g.kind(t) != k) return nullptr;
    return &m_g.node(t).fields;
  }

  static int field_type(const std::vector<CandidGraphField> *fields,
                        uint32_t id) {
    if (fields)
      for (const auto &f : *fields)
        if (f.id == id) return f.type;
    return INT32_MIN;
  }

  // The content of an opt or vec, of its type, or of any type when t is
  // not one
  CandidValue child(int t, size_t depth) {
    if (CandidTypeGraph::is_node(t)) {
      CandidKind k = m_g.kind(t);
      if (k == CandidKind::Opt || k == CandidKind::Vec)
        return value(m_g.node(t).inner, depth + 1);
    }
    return value(CandidTypeGraph::prim(CandidKind::Nat), depth + 1);
  }

  void vec(int t, CandidValue &v, uint8_t mutate, bool stop, size_t depth) {
    if (mutate % 8 == 1) {
      // Packed: of a kind and bytes that may not fit
      v.packed = true;
      v.elem_kind = ALL_KINDS[m_r.byte() % std::size(ALL_KINDS)];
      for (size_t k = 0, n = m_r.byte(); k < n && !m_r.done(); ++k)
        v.bytes.push_back(m_r.byte());
      return;
    }
    if (stop) return;
    if (mutate % 8 == 2) {
      // Repeated: one element, many times
      v.items.push_back(child(t, depth));
      v.repeat = size_t(m_r.u64() % 3000000);
      return;
    }
    for (size_t k = 0, n = m_r.byte() % 8; k < n && !m_r.done(); ++k)
      v.items.push_back(child(t, depth));
  }

  void record(int t, CandidValue &v, uint8_t mutate, bool stop, size_t depth) {
    const auto *fields = node_fields(t, CandidKind::Record);
    if (fields)
      for (const auto &f : *fields) {
        uint8_t b = m_r.byte();
        if (b < 8) continue; // a field left out
        v.ids.push_back(b < 16 ? uint32_t(m_r.u64()) : f.id);
        v.items.push_back(stop ? CandidValue{} : value(f.type, depth + 1));
      }
    if (mutate % 8 == 3) {
      // An extra field, maybe out of order
      v.ids.push_back(uint32_t(m_r.u64()));
      v.items.push_back(CandidValue{});
    }
    if (mutate % 8 == 4 && !v.ids.empty()) {
      // Ids and items of different lengths
      v.ids.pop_back();
    }
    if (mutate % 8 == 5 && fields) {
      // Some fields in rest, as the decoder shares them
      auto rest = std::make_shared<CandidValue>();
      rest->kind = CandidKind::Record;
      while (!v.ids.empty() && (m_r.byte() & 1)) {
        rest->ids.insert(rest->ids.begin(), v.ids.back());
        rest->items.insert(rest->items.begin(), v.items.back());
        v.ids.pop_back();
        v.items.pop_back();
      }
      v.rest = rest;
    }
  }
};

[[noreturn]] void fail(const std::string &what) {
  std::fprintf(stderr, "fuzz_encoder: %s\n", what.c_str());
  std::abort();
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  static const std::vector<FuzzExpected> expected = make_fuzz_expected();
  if (size < 1) return 0;
  const FuzzExpected &e = expected[data[0] % expected.size()];
  Reader r(data + 1, size - 1);
  Builder b(e.g, r);
  std::vector<CandidValue> values;
  for (int t : e.args)
    values.push_back(b.value(t, 0));

  // A smaller budget than the default 1M: a repeated element of a few bytes
  // makes a few bytes of output per repeat, and the round trip a CandidValue
  // per element, within libFuzzer's -malloc_limit_mb
  CandidDecodeLimits limits;
  limits.max_zero_width_values = 20000;

  std::vector<uint8_t> once;
  std::string err;
  MemoryOracle::start();
  bool ok = CandidEncoder::encode(e.g, e.args, values, once, err, limits);
  if (!ok) return 0;
  // The encoder holds the bytes it writes, and little more. Only checked when
  // it succeeds: a refused value can be refused after a large part of the
  // message was written, and those bytes are gone
  MemoryOracle::check(size + once.size(), "CandidEncoder::encode");

  // What the encoder writes, the decoder reads, and it encodes the same
  std::vector<CandidValue> again;
  if (!CandidDecoder::decode(once.data(), once.size(), e.g, e.args, again, err,
                             limits))
    fail("an encoded value does not decode: " + err);
  std::vector<uint8_t> twice;
  if (!CandidEncoder::encode(e.g, e.args, again, twice, err, limits))
    fail("a decoded value does not encode: " + err);
  if (twice != once) fail("a value encodes to other bytes the second time");
  return 0;
}
