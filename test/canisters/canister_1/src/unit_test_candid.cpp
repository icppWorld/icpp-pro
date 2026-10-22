// Unit tests for ic/candid/*.cpp

#include "unit_test_candid.h"
#include "candid_type.h"

#include "candid_type_all_includes.h"

#include "candid_type_visitors.h"

#include <algorithm>
#include <limits>
#include <optional>
#include <string>

#include "candid_deserialize.h"
#include "candid_serialize.h"
#include "candid_serialize_type_table_registry.h"
#include "ic_api.h"
#include "test_vec_of.h"

namespace {
// The hex of a record with the fields 10 (nat) & 20 (text), and additional
// fields of many types before, between and after them. wrap puts the record
// in a variant, an opt or a vec.
enum class Wrap { None, Variant, Opt };
std::string superset_hex(__uint128_t n, const std::string &t,
                         Wrap wrap = Wrap::None) {
  // The type table registry must be cleared before the types are built
  CandidSerializeTypeTableRegistry::get_instance().clear();
  CandidTypeRecord inner;
  inner.append(1, CandidTypeText("inner"));
  inner.append(2, CandidTypeVecNat8(std::vector<uint8_t>{1, 2, 3}));

  CandidTypeRecord r;
  r.append(3, CandidTypeOptRecord(inner));
  r.append(5, CandidTypeNat(__uint128_t(5)));
  r.append(10, CandidTypeNat(n));
  r.append(15, CandidTypeText("between"));
  r.append(16, inner);
  r.append(17, CandidTypeVariant("x", CandidTypeInt(__int128_t(-7))));
  r.append(20, CandidTypeText(t));
  r.append(25, CandidTypeVecNat(std::vector<__uint128_t>{1, 2}));
  r.append(30, CandidTypeVecNat8(std::vector<uint8_t>{0xde, 0xad}));
  r.append(35, CandidTypePrincipal("aaaaa-aa"));
  r.append(40, CandidTypeRecord());
  r.append(45, CandidTypeBool(true));
  if (wrap == Wrap::Variant)
    return CandidSerialize(CandidTypeVariant("ok", r)).as_hex_string();
  if (wrap == Wrap::Opt)
    return CandidSerialize(CandidTypeOptRecord(r)).as_hex_string();
  return CandidSerialize(r).as_hex_string();
}

void decode(const std::string &hex_string, const CandidType &expected) {
  CandidArgs A;
  A.append(expected);
  CandidDeserialize(hex_string, A);
}

void check(bool ok, const std::string &what) {
  if (!ok) IC_API::trap("unit_test_candid: " + what);
}

// The test functions are noinline: inlined into unit_test_candid, their locals
// make one stack frame larger than the 64 KiB wasm stack
__attribute__((noinline)) void test_record_width_subtyping() {
  { // A record
    std::string wire = superset_hex(42, "hello");
    __uint128_t n{0};
    std::string t;
    CandidTypeRecord r;
    r.append(10, CandidTypeNat(&n));
    r.append(20, CandidTypeText(&t));
    decode(wire, r);
    check(n == 42 && t == "hello", "record");
  }

  { // A record in a variant case
    std::string wire = superset_hex(43, "in variant", Wrap::Variant);
    __uint128_t n{0};
    std::string t;
    std::string label;
    CandidTypeRecord r;
    r.append(10, CandidTypeNat(&n));
    r.append(20, CandidTypeText(&t));
    CandidTypeVariant v{&label};
    v.append("ok", r);
    decode(wire, v);
    check(label == "ok" && n == 43 && t == "in variant", "variant");
  }

  { // A record in an opt
    std::string wire = superset_hex(44, "in opt", Wrap::Opt);
    __uint128_t n{0};
    std::string t;
    bool has_value{false};
    CandidTypeRecord r;
    r.append(10, CandidTypeNat(&n));
    r.append(20, CandidTypeText(&t));
    decode(wire, CandidTypeOptRecord(&r, &has_value));
    check(has_value && n == 44 && t == "in opt", "opt");
  }

  { // Records in a vec
    CandidSerializeTypeTableRegistry::get_instance().clear();
    CandidTypeRecord w;
    w.append(3, CandidTypeVecNat(std::vector<__uint128_t>{7, 8}));
    w.append(10, CandidTypeVecNat(std::vector<__uint128_t>{45, 46}));
    w.append(15, CandidTypeVecText(std::vector<std::string>{"x", "y"}));
    w.append(20, CandidTypeVecText(std::vector<std::string>{"a", "b"}));
    w.append(25, CandidTypeVecBool(std::vector<bool>{true, false}));
    std::string wire = CandidSerialize(CandidTypeVecRecord(w)).as_hex_string();

    std::vector<__uint128_t> ns;
    std::vector<std::string> ts;
    CandidTypeRecord r;
    r.append(10, CandidTypeVecNat(&ns));
    r.append(20, CandidTypeVecText(&ts));
    decode(wire, CandidTypeVecRecord(&r));
    check(ns == std::vector<__uint128_t>{45, 46} &&
              ts == std::vector<std::string>{"a", "b"},
          "vec");
  }

  { // An expected opt that is not on the wire, after an additional field
    std::string wire = superset_hex(47, "no opt");
    __uint128_t n{0};
    std::string t;
    std::optional<std::string> o;
    CandidTypeRecord r;
    r.append(10, CandidTypeNat(&n));
    r.append(20, CandidTypeText(&t));
    r.append(99, CandidTypeOptText(&o));
    decode(wire, r);
    check(n == 47 && t == "no opt" && !o.has_value(), "missing opt");
  }

  { // A record with only additional fields
    std::string wire = superset_hex(48, "all extra");
    decode(wire, CandidTypeRecord());
  }

  // The tests that follow expect an empty type table registry
  CandidSerializeTypeTableRegistry::get_instance().clear();
}

// A vec of variants: each element is matched by the id (hash) of its case,
// so the wire may have more cases than expected, in any order, and a skipped
// vec of variants reads the payloads.
__attribute__((noinline)) void test_vec_variant() {
  { // didc encode -t '(vec variant {a; b; c})' '(vec {variant {b}; variant {c}; variant {b}})'
    std::string wire = "4449444c026d016b03617f627f637f010003010201";
    CandidSerializeTypeTableRegistry::get_instance().clear();
    std::vector<std::string> labels;
    CandidTypeVariant v;
    v.append("c", CandidTypeNull{});
    v.append("b", CandidTypeNull{});
    decode(wire, CandidTypeVecVariant(&v, &labels));
    check(labels == std::vector<std::string>{"b", "c", "b"},
          "vec variant with more cases on the wire");
  }

  { // didc encode -t '(record {x: vec variant {a: nat; b: text}; y: nat})' '(record {x = vec {variant {a = 5}; variant {b = "hi"}}; y = 7})'
    std::string wire =
        "4449444c036c027801797d6d026b02617d627101000200050102686907";
    CandidSerializeTypeTableRegistry::get_instance().clear();
    __uint128_t y{0};
    CandidTypeRecord r;
    r.append("y", CandidTypeNat(&y));
    decode(wire, r);
    check(y == 7, "skipped vec variant with payloads");
  }

  CandidSerializeTypeTableRegistry::get_instance().clear();
}

// A record field added with append_optional may be absent on the wire: it is
// then left as is, and *p_on_wire tells whether it was on the wire.
__attribute__((noinline)) void test_append_optional() {
  { // On the wire
    // didc encode -t '(record {a: nat; b: text})' '(record {a = 1; b = "x"})'
    std::string wire = "4449444c016c02617d62710100010178";
    CandidSerializeTypeTableRegistry::get_instance().clear();
    __uint128_t a{0};
    bool a_on_wire{false};
    std::string b;
    CandidTypeRecord r;
    r.append_optional("a", CandidTypeNat(&a), &a_on_wire);
    r.append("b", CandidTypeText(&b));
    decode(wire, r);
    check(a == 1 && a_on_wire && b == "x", "append_optional on the wire");
  }

  { // Absent, first field
    // didc encode -t '(record {b: text})' '(record {b = "x"})'
    std::string wire = "4449444c016c01627101000178";
    CandidSerializeTypeTableRegistry::get_instance().clear();
    __uint128_t a{11};
    bool a_on_wire{true};
    std::string b;
    CandidTypeRecord r;
    r.append_optional("a", CandidTypeNat(&a), &a_on_wire);
    r.append("b", CandidTypeText(&b));
    decode(wire, r);
    check(a == 11 && !a_on_wire && b == "x", "append_optional absent");
  }

  { // Absent, after an additional field
    // didc encode -t '(record {a: nat; c: text})' '(record {a = 9; c = "y"})'
    std::string wire = "4449444c016c02617d63710100090179";
    CandidSerializeTypeTableRegistry::get_instance().clear();
    std::string b{"unchanged"};
    std::string c;
    CandidTypeRecord r;
    r.append_optional("b", CandidTypeText(&b));
    r.append("c", CandidTypeText(&c));
    decode(wire, r);
    check(b == "unchanged" && c == "y",
          "append_optional absent after an additional field");
  }

  { // Absent, last field
    // didc encode -t '(record {a: nat})' '(record {a = 3})'
    std::string wire = "4449444c016c01617d010003";
    CandidSerializeTypeTableRegistry::get_instance().clear();
    __uint128_t a{0};
    std::vector<std::string> z{"unchanged"};
    bool z_on_wire{true};
    CandidTypeRecord r;
    r.append("a", CandidTypeNat(&a));
    r.append_optional("z", CandidTypeVecText(&z), &z_on_wire);
    decode(wire, r);
    check(a == 3 && !z_on_wire && z == std::vector<std::string>{"unchanged"},
          "append_optional absent last");
  }

  { // Encodes exactly like append
    CandidSerializeTypeTableRegistry::get_instance().clear();
    CandidTypeRecord r1;
    r1.append("a", CandidTypeNat(__uint128_t(1)));
    r1.append("b", CandidTypeText("x"));
    std::string hex1 = CandidSerialize(r1).as_hex_string();
    CandidSerializeTypeTableRegistry::get_instance().clear();
    CandidTypeRecord r2;
    r2.append_optional("a", CandidTypeNat(__uint128_t(1)));
    r2.append("b", CandidTypeText("x"));
    std::string hex2 = CandidSerialize(r2).as_hex_string();
    check(hex1 == "4449444c016c02617d62710100010178" && hex2 == hex1,
          "append_optional encodes like append");
  }

  CandidSerializeTypeTableRegistry::get_instance().clear();
}

// A type table index is a sleb128 (Candid: T(<datatype>) = sleb128(i)). An
// index of 64 or more needs 2 bytes in sleb128, and differs from uleb128.
__attribute__((noinline)) void test_type_table_index_64() {
  CandidSerializeTypeTableRegistry::get_instance().clear();
  // Fill the registry with 64 distinct type tables
  std::vector<CandidTypeRecord> fillers(64);
  for (uint32_t k = 0; k < 64; ++k) {
    fillers[k].append(k, CandidTypeNat(__uint128_t(k)));
  }
  // Every type that refers to another type table: the argument itself, a
  // record field, a variant case, an opt record, an opt variant, an opt vec
  CandidTypeRecord inner;
  inner.append("q", CandidTypeText("deep"));
  CandidTypeRecord outer;
  outer.append("p", inner);
  outer.append("v", CandidTypeVariant("c", inner));
  outer.append("o", CandidTypeOptRecord(inner));
  outer.append("w", CandidTypeOptVariant(CandidTypeVariant("d", inner)));
  outer.append("x",
               CandidTypeOptVec(CandidTypeVecNat8(std::vector<uint8_t>{7})));
  std::string wire = CandidSerialize(outer).as_hex_string();

  CandidSerializeTypeTableRegistry::get_instance().clear();
  std::string q_p, q_v, q_o, q_w;
  std::string label_v, label_w;
  bool has_o{false}, has_w{false}, has_x{false};
  std::vector<uint8_t> x;
  CandidTypeRecord inner_p;
  inner_p.append("q", CandidTypeText(&q_p));
  CandidTypeRecord inner_v;
  inner_v.append("q", CandidTypeText(&q_v));
  CandidTypeRecord inner_o;
  inner_o.append("q", CandidTypeText(&q_o));
  CandidTypeRecord inner_w;
  inner_w.append("q", CandidTypeText(&q_w));
  CandidTypeVariant v{&label_v};
  v.append("c", inner_v);
  CandidTypeVariant w{&label_w};
  w.append("d", inner_w);
  CandidTypeVecNat8 x_vec{&x};
  CandidTypeRecord outer_d;
  outer_d.append("p", inner_p);
  outer_d.append("v", v);
  outer_d.append("o", CandidTypeOptRecord(&inner_o, &has_o));
  outer_d.append("w", CandidTypeOptVariant(&w, &has_w));
  outer_d.append("x", CandidTypeOptVec(&x_vec, &has_x));
  decode(wire, outer_d);
  check(q_p == "deep" && label_v == "c" && q_v == "deep" && has_o &&
            q_o == "deep" && has_w && label_w == "d" && q_w == "deep" &&
            has_x && x == std::vector<uint8_t>{7},
        "a type table index of 64 or more");
  CandidSerializeTypeTableRegistry::get_instance().clear();
}

// CandidTypeVecOf: a vec of any element type
std::vector<VecOfItem> decode_vec_of_items(const std::string &wire) {
  CandidSerializeTypeTableRegistry::get_instance().clear();
  VecOfItem item;
  std::vector<VecOfItem> items;
  decode(wire, CandidTypeVecOf(
                   [&]() -> CandidType {
                     item = {};
                     return vec_of_item_type(item);
                   },
                   [&]() { items.push_back(item); }));
  return items;
}

const char *VEC_OF_A2 =
    "4449444c066d016c05dbb70178d4c2a7b80402f2afa8c80403d9e9dae7040496e3f0f00b05"
    "6b02617f627d6e716d716c01717101000201000000000000000001026e3102017801790271"
    "31020000000000000001ac020000027132";

// Encodes n elements record { k: nat64; o: opt nat } with CandidTypeVecOf,
// decodes and checks them, and returns the instructions of the decode
__attribute__((noinline)) uint64_t encode_decode_k(uint64_t n) {
  CandidSerializeTypeTableRegistry::get_instance().clear();
  std::vector<CandidType> elements;
  for (uint64_t k = 0; k < n; ++k) {
    CandidTypeRecord r;
    r.append("k", CandidTypeNat64(k));
    r.append("o", CandidTypeOptNat(k % 2 ? std::optional<__uint128_t>(k)
                                         : std::nullopt));
    elements.push_back(r);
  }
  std::string wire =
      CandidSerialize(CandidTypeVecOf(elements[0], elements)).as_hex_string();
  CandidSerializeTypeTableRegistry::get_instance().clear();
  uint64_t k{0};
  std::optional<__uint128_t> o;
  uint64_t count{0};
  bool ok{true};
  uint64_t start = IC_API::performance_counter();
  decode(wire, CandidTypeVecOf(
                   [&]() -> CandidType {
                     o.reset();
                     CandidTypeRecord r;
                     r.append("k", CandidTypeNat64(&k));
                     r.append("o", CandidTypeOptNat(&o));
                     return r;
                   },
                   [&]() {
                     ok = ok && k == count &&
                          o.has_value() == bool(count % 2) &&
                          (!o || *o == count);
                     ++count;
                   }));
  uint64_t instructions = IC_API::performance_counter() - start;
  check(ok && count == n, "vec_of " + std::to_string(n) + " elements");
  return instructions;
}

__attribute__((noinline)) void test_vec_of_decode() {
  // didc encode -t '(vec record { id: nat64; note: opt text; kind: variant { a; b: nat }; tags: vec text; inner: record { q: text } })'
  { // 2 elements: '(vec { record { id = 1; note = opt "n1"; kind = variant { a }; tags = vec { "x"; "y" }; inner = record { q = "q1" } }; record { id = 2; note = null; kind = variant { b = 300 }; tags = vec {}; inner = record { q = "q2" } } })'
    auto items = decode_vec_of_items(VEC_OF_A2);
    check(items.size() == 2, "vec_of A2 size");
    check(items[0].id == 1 && items[0].note == "n1" && items[0].kind == "a" &&
              items[0].tags == std::vector<std::string>{"x", "y"} &&
              items[0].q == "q1",
          "vec_of A2 element 0");
    check(items[1].id == 2 && !items[1].note && items[1].kind == "b" &&
              items[1].amount == 300 && items[1].tags.empty() &&
              items[1].q == "q2",
          "vec_of A2 element 1");
  }
  { // 0 elements: '(vec {})'
    auto items = decode_vec_of_items(
        "4449444c066d016c05dbb70178d4c2a7b80402f2afa8c80403d9e9dae7040496e3f0f0"
        "0b056b02617f627d6e716d716c017171010000");
    check(items.empty(), "vec_of A0");
  }
  { // 1 element: '(vec { record { id = 7; note = null; kind = variant { b = 5 }; tags = vec { "t" }; inner = record { q = "" } } })'
    auto items = decode_vec_of_items(
        "4449444c066d016c05dbb70178d4c2a7b80402f2afa8c80403d9e9dae7040496e3f0f0"
        "0b056b02617f627d6e716d716c0171710100010700000000000000010500010174"
        "00");
    check(items.size() == 1 && items[0].id == 7 && !items[0].note &&
              items[0].kind == "b" && items[0].amount == 5 &&
              items[0].tags == std::vector<std::string>{"t"} &&
              items[0].q.empty(),
          "vec_of A1");
  }

  { // Variants with payloads, every kind of payload & a unit case
    // didc encode -t '(vec variant { i32: int32; f64: float64; v128: nat; t: text; u })' '(vec { variant { i32 = -2147483648 }; variant { f64 = 1.5 }; variant { v128 = 340282366920938463463374607431768211455 }; variant { t = "hi" }; variant { u } })'
    std::string wire =
        "4449444c026d016b057471757fa4aab60272c8b2bf0275818f92f1047d01000503000000"
        "8002000000000000f83f04ffffffffffffffffffffffffffffffffffff0300026869"
        "01";
    CandidSerializeTypeTableRegistry::get_instance().clear();
    struct G {
      std::string kind;
      int32_t i32{0};
      double f64{0};
      __uint128_t v128{0};
      std::string t;
    } g;
    std::vector<G> gs;
    decode(wire, CandidTypeVecOf(
                     [&]() -> CandidType {
                       g = {};
                       // the cases appended in an order different from the wire
                       CandidTypeVariant v{&g.kind};
                       v.append("u", CandidTypeNull{});
                       v.append("v128", CandidTypeNat(&g.v128));
                       v.append("t", CandidTypeText(&g.t));
                       v.append("i32", CandidTypeInt32(&g.i32));
                       v.append("f64", CandidTypeFloat64(&g.f64));
                       return v;
                     },
                     [&]() { gs.push_back(g); }));
    check(gs.size() == 5 && gs[0].kind == "i32" &&
              gs[0].i32 == std::numeric_limits<int32_t>::min() &&
              gs[1].kind == "f64" && gs[1].f64 == 1.5 && gs[2].kind == "v128" &&
              gs[2].v128 == ~__uint128_t(0) && gs[3].kind == "t" &&
              gs[3].t == "hi" && gs[4].kind == "u",
          "vec_of variants with payloads");
  }

  { // Two levels: a vec of records, each holding a vec of records
    // didc encode -t '(vec record { ts: nat64; items: vec record { id: principal; n: nat64; m: opt nat } })' '(vec { record { ts = 10; items = vec { record { id = principal "aaaaa-aa"; n = 1; m = opt 9 }; record { id = principal "2vxsx-fae"; n = 2; m = null } } }; record { ts = 20; items = vec {} } })'
    std::string wire =
        "4449444c056d016c02ffca017880d8b2900c026d036c036d046e78dbb701686e7d01000"
        "20a0000000000000002010901000000000000000100000200000000000000010104140"
        "000000000000000";
    CandidSerializeTypeTableRegistry::get_instance().clear();
    struct Item {
      std::string id;
      uint64_t n{0};
      std::optional<__uint128_t> m;
    } it;
    struct Entry {
      uint64_t ts{0};
      std::vector<Item> items;
    } e;
    std::vector<Entry> es;
    decode(wire, CandidTypeVecOf(
                     [&]() -> CandidType {
                       e = {};
                       CandidTypeRecord r;
                       r.append("ts", CandidTypeNat64(&e.ts));
                       r.append("items",
                                CandidTypeVecOf(
                                    [&]() -> CandidType {
                                      it = {};
                                      CandidTypeRecord ri;
                                      ri.append("id",
                                                CandidTypePrincipal(&it.id));
                                      ri.append("n", CandidTypeNat64(&it.n));
                                      ri.append("m", CandidTypeOptNat(&it.m));
                                      return ri;
                                    },
                                    [&]() { e.items.push_back(it); }));
                       return r;
                     },
                     [&]() { es.push_back(e); }));
    check(es.size() == 2 && es[0].ts == 10 && es[0].items.size() == 2 &&
              es[0].items[0].id == "aaaaa-aa" && es[0].items[0].n == 1 &&
              es[0].items[0].m == __uint128_t(9) &&
              es[0].items[1].id == "2vxsx-fae" && es[0].items[1].n == 2 &&
              !es[0].items[1].m && es[1].ts == 20 && es[1].items.empty(),
          "vec_of two levels");
  }

  { // A vec as a variant case
    // didc encode -t '(variant { ok: vec record { x: opt nat }; err: text })' '(variant { ok = vec { record { x = opt 1 }; record { x = null } } })'
    std::string wire =
        "4449444c046b029cc20101e58eb402716d026c0178036e7d01000002010100";
    CandidSerializeTypeTableRegistry::get_instance().clear();
    std::string label;
    std::string err;
    std::optional<__uint128_t> x;
    std::vector<std::optional<__uint128_t>> xs;
    CandidTypeVariant v{&label};
    v.append("ok", CandidTypeVecOf(
                       [&]() -> CandidType {
                         x.reset();
                         CandidTypeRecord r;
                         r.append("x", CandidTypeOptNat(&x));
                         return r;
                       },
                       [&]() { xs.push_back(x); }));
    v.append("err", CandidTypeText(&err));
    decode(wire, v);
    check(label == "ok" && xs.size() == 2 && xs[0] == __uint128_t(1) && !xs[1],
          "vec_of as a variant case");
  }

  { // A vec of variants in an opt record
    // didc encode -t '(opt record { list: vec variant { a: nat; b } })' '(opt record { list = vec { variant { b }; variant { a = 4 } } })'
    std::string wire =
        "4449444c046e016c01deb8ccbd04026d036b02617d627f01000102010004";
    CandidSerializeTypeTableRegistry::get_instance().clear();
    bool has_value{false};
    std::string kind;
    __uint128_t a{0};
    std::vector<std::pair<std::string, __uint128_t>> list;
    CandidTypeRecord r;
    r.append("list", CandidTypeVecOf(
                         [&]() -> CandidType {
                           kind.clear();
                           a = 0;
                           CandidTypeVariant v{&kind};
                           v.append("a", CandidTypeNat(&a));
                           v.append("b", CandidTypeNull{});
                           return v;
                         },
                         [&]() { list.push_back({kind, a}); }));
    decode(wire, CandidTypeOptRecord(&r, &has_value));
    check(has_value && list.size() == 2 && list[0].first == "b" &&
              list[1].first == "a" && list[1].second == 4,
          "vec_of in an opt record");
  }

  { // Primitive elements
    // didc encode -t '(vec nat64)' '(vec { 1; 18446744073709551615 })'
    CandidSerializeTypeTableRegistry::get_instance().clear();
    uint64_t n{0};
    std::vector<uint64_t> ns;
    decode("4449444c016d780100020100000000000000ffffffffffffffff",
           CandidTypeVecOf([&]() -> CandidType { return CandidTypeNat64(&n); },
                           [&]() { ns.push_back(n); }));
    check(ns == std::vector<uint64_t>{1, std::numeric_limits<uint64_t>::max()},
          "vec_of nat64");

    // didc encode -t '(vec principal)' '(vec { principal "aaaaa-aa"; principal "2vxsx-fae" })'
    CandidSerializeTypeTableRegistry::get_instance().clear();
    std::string p;
    std::vector<std::string> ps;
    decode(
        "4449444c016d680100020100010104",
        CandidTypeVecOf([&]() -> CandidType { return CandidTypePrincipal(&p); },
                        [&]() { ps.push_back(p); }));
    check(ps == std::vector<std::string>{"aaaaa-aa", "2vxsx-fae"},
          "vec_of principal");
  }

  { // Width subtyping in the elements: additional fields before, between and
    // after the expected ones, an additional vec of variants with payloads,
    // and an expected opt that is null
    // didc encode -t '(vec record { a: nat; extra1: text; b: opt text; extra2: vec variant { p: nat; q }; c: text })' '(vec { record { a = 1; extra1 = "e"; b = opt "B"; extra2 = vec { variant { p = 3 }; variant { q } }; c = "c1" }; record { a = 2; extra1 = "f"; b = null; extra2 = vec {}; c = "c2" } })'
    std::string wire =
        "4449444c056d016c05617d62026371a1c792b90c71a2c792b90c036e716d046b02707d71"
        "7f010002010101420263310165020003010200026332016600";
    CandidSerializeTypeTableRegistry::get_instance().clear();
    __uint128_t a{0};
    std::optional<std::string> b;
    std::string c;
    std::optional<std::string> z;
    std::vector<std::string> out;
    decode(wire, CandidTypeVecOf(
                     [&]() -> CandidType {
                       a = 0;
                       b.reset();
                       c.clear();
                       z.reset();
                       CandidTypeRecord r;
                       r.append("a", CandidTypeNat(&a));
                       r.append("b", CandidTypeOptText(&b));
                       r.append("c", CandidTypeText(&c));
                       r.append("z", CandidTypeOptText(&z));
                       return r;
                     },
                     [&]() {
                       out.push_back(std::to_string(uint64_t(a)) + "," +
                                     b.value_or("-") + "," + c + "," +
                                     z.value_or("-"));
                     }));
    check(out == std::vector<std::string>{"1,B,c1,-", "2,-,c2,-"},
          "vec_of width subtyping in the elements");
  }

  { // 1,000 elements, and the decode is linear in the number of elements
    uint64_t instructions_100 = encode_decode_k(100);
    uint64_t instructions_1000 = encode_decode_k(1000);
    check(instructions_1000 < 20 * instructions_100,
          "vec_of 1,000 elements decode is not linear: " +
              std::to_string(instructions_100) + " for 100, " +
              std::to_string(instructions_1000) + " for 1,000");
  }

  CandidSerializeTypeTableRegistry::get_instance().clear();
}

// An additional field of a vec of records with opt & variant fields, or of a
// vec of change records (canister_info), is read and discarded
__attribute__((noinline)) void test_skip_vec_of() {
  { // didc encode -t '(record { a : nat; extra : vec record { x : opt nat; y : variant { p : record { q : text }; r } }; z : text })' '(record { a = 2; extra = vec { record { x = opt 1; y = variant { p = record { q = "q" } } }; record { x = null; y = variant { r } } }; z = "after" })'
    std::string wire =
        "4449444c066c03617d7a7190b58ab907016d026c02780379046e7d6b027005727f6c0171"
        "710100020561667465720201010001710001";
    CandidSerializeTypeTableRegistry::get_instance().clear();
    __uint128_t a{0};
    std::string z;
    CandidTypeRecord r;
    r.append("a", CandidTypeNat(&a));
    r.append("z", CandidTypeText(&z));
    decode(wire, r);
    check(a == 2 && z == "after", "skip a vec of records");
  }
  { // record { a : nat; changes : vec change; z : text }, with change as in
    // the replica's ic.did, its 7 detail cases & both origins
    std::string wire =
        "4449444c166c03617d7a7183e2b2e60c016d026c04d6f68e800178c0c3dff50278e6b384"
        "d80403c2b9dbda0a076b0280d1e8900204dced83b40b056c018fc1d4fb06686c02c0c3df"
        "f50206b3c4b1f204686e786b079f90dedf0208d798fbac040cbd81f4ce040e8cb1bb9a05"
        "11c89cb4c90a1398f795de0a1590c6909a0f7f6c02d7e09b900209c2e7e2ce0b0a6d686e"
        "0b6d7b6c02e3a683c3040d81cfaef40a0b6b03c8bb8a707f9ce9c699067f9baaebec087f"
        "6c05c0c3dff50278dbe1c691030f9ed9c0ad081082bff3a50d78b6b897890f0b6b0291a0"
        "c58d0370a4f794e508706e686c02d7e09b900212c2e7e2ce0b0a6e096c04dca5dce70214"
        "b3c4b1f20468c8b6fa9307688fedd8b10e786c03b3c4b1f2046898cec7e707788fedd8b1"
        "0e786c01d7e09b9002090100010561667465720701000000000000000000000000000000"
        "000101040001010001020102020000000000000001000000000000000101050000000000"
        "00000100010101ab03000000000000000200000000000000010001000604000000000000"
        "000300000000000000000100020200000000000000010009000000000000000200010500"
        "000000000000040000000000000000010005000600000000000000050000000000000000"
        "010003010101010400070000000000000006000000000000000001000401010401000000"
        "000000000400000000000000010001000300000000000000";
    CandidSerializeTypeTableRegistry::get_instance().clear();
    __uint128_t a{0};
    std::string z;
    CandidTypeRecord r;
    r.append("a", CandidTypeNat(&a));
    r.append("z", CandidTypeText(&z));
    decode(wire, r);
    check(a == 1 && z == "after", "skip a vec of change records");
  }
  CandidSerializeTypeTableRegistry::get_instance().clear();
}
} // namespace

int unit_test_candid() {
  { // Verify idl hash
    CandidSerializeTypeTableRegistry::get_instance().clear();
    if (CandidTypeBase<CandidTypeRecord>().idl_hash("syndactyle") != 4260381820)
      IC_API::trap(std::string(__func__) + ": 1a");
    if (CandidTypeBase<CandidTypeRecord>().idl_hash("rectum") != 4260381820)
      IC_API::trap(std::string(__func__) + ": 1b");
    if (CandidTypeBase<CandidTypeRecord>().idl_hash("trustbuster") !=
        3504418361)
      IC_API::trap(std::string(__func__) + ": 1c");
    if (CandidTypeBase<CandidTypeRecord>().idl_hash("destroys") != 3504418361)
      IC_API::trap(std::string(__func__) + ": 1d");
  }
  {
    // Verify that the CandidType '<' operator enables sorting
    // This does not compile to wasm
    // CandidSerializeTypeTableRegistry::get_instance().clear();
    // std::vector<CandidType> vec;
    // vec.push_back(CandidTypeBool(true));
    // vec.push_back(CandidTypeInt(42));
    // vec.push_back(CandidTypeText("hello!"));
    // std::sort(vec.begin(), vec.end());
    // CandidTypeRoot *ptr_prev = get_candid_type_root_ptr(vec[0]);
    // int opcode_prev = ptr_prev->get_datatype_opcode();
    // for (CandidType v : vec) {
    //   CandidTypeRoot *ptr_v = get_candid_type_root_ptr(v);
    //   int opcode = ptr_v->get_datatype_opcode();
    //   if (opcode_prev > opcode) IC_API::trap(std::string(__func__) + ": 2");
    //   opcode_prev = opcode;
    // }
  }

  { // Verify the VecBytes static utils
    CandidSerializeTypeTableRegistry::get_instance().clear();
    const std::byte b = std::byte(42);
    if (VecBytes::byte_to_hex(b) != "0x2a")
      IC_API::trap(std::string(__func__) + ": 3a");

    if (VecBytes::byte_to_hex(b, "") != "2a")
      IC_API::trap(std::string(__func__) + ": 3b");

    if (VecBytes::byte_to_dec(b) != "      42")
      IC_API::trap(std::string(__func__) + ": 3c");

    if (VecBytes::byte_to_dec(b, 2) != "42")
      IC_API::trap(std::string(__func__) + ": 3d");

    if (VecBytes::byte_to_char(b) != "*")
      IC_API::trap(std::string(__func__) + ": 3e");

    if (IC_API::to_string_128(__uint128_t(101)) != "101")
      IC_API::trap(std::string(__func__) + ": 3f");

    if (IC_API::to_string_128(__int128_t(101)) != "101")
      IC_API::trap(std::string(__func__) + ": 3f");

    if (IC_API::to_string_128(__int128_t(-101)) != "-101")
      IC_API::trap(std::string(__func__) + ": 3f");

    {
      std::string max_uint128 = "340282366920938463463374607431768211455";
      __uint128_t i = VecBytes::atouint128_t(max_uint128);
      if (IC_API::to_string_128(i) != max_uint128)
        IC_API::trap(std::string(__func__) + ": 3g");
    }
  }
  { // Verify serialization & deserializtion of 2 records
    // didc encode '(record { 6 = 42 : int; 9 = 43 : int }, record { 7 = 44 : int; 10 = 45 : int })'

    { // serialize

      CandidSerializeTypeTableRegistry::get_instance().clear();
      CandidTypeRecord r1;
      r1.append(6, CandidTypeInt(42));
      r1.append(9, CandidTypeInt(43));
      //
      CandidTypeRecord r2;
      r2.append(7, CandidTypeInt(44));
      r2.append(10, CandidTypeInt(45));
      //
      CandidArgs A;
      A.append(r1);
      A.append(r2);
      //
      if (CandidSerialize(A).assert_candid(
              "4449444c026c02067c097c6c02077c0a7c0200012a2b2c2d", true))
        IC_API::trap(std::string(__func__) + ": 4a");
    }

    { // deserialize
      CandidSerializeTypeTableRegistry::get_instance().clear();
      __int128_t r1_i1;
      __int128_t r1_i2;
      CandidTypeRecord r1;
      r1.append(6, CandidTypeInt(&r1_i1));
      r1.append(9, CandidTypeInt(&r1_i2));
      //
      __int128_t r2_i1;
      __int128_t r2_i2;
      CandidTypeRecord r2;
      r2.append(7, CandidTypeInt(&r2_i1));
      r2.append(10, CandidTypeInt(&r2_i2));
      //
      CandidArgs A;
      A.append(r1);
      A.append(r2);
      //
      CandidDeserialize("4449444c026c02067c097c6c02077c0a7c0200012a2b2c2d", A);
      if (r1_i1 != 42 || r1_i2 != 43 || r2_i1 != 44 || r2_i2 != 45)
        IC_API::trap(std::string(__func__) + ": 4b");
      // std::string name = record_in["name"];
      // double secret_x = record_in["secret float64"];
      // int secret_i = record_in["secret int"];}
    }
  }

  { // Verify serialization variant of tiny Stories example
    // didc encode '(variant { ok = "Once upon a time, there was a little girl named Lily. She loved to play outside in the sunshine. One day, she saw a big, red ball in the sky. It was the sun! She thought it was so pretty.\nLily wanted to play with the ball, but it was too high up in the sky. She tried to jump and reach it, but she couldn't. Then, she had an idea. She would use a stick to knock the" : text })'

    CandidSerializeTypeTableRegistry::get_instance().clear();
    CandidArgs A;
    std::string output =
        "Once upon a time, there was a little girl named Lily. She loved to play outside in the sunshine. One day, she saw a big, red ball in the sky. It was the sun! She thought it was so pretty.\nLily wanted to play with the ball, but it was too high up in the sky. She tried to jump and reach it, but she couldn\'t. Then, she had an idea. She would use a stick to knock the";
    A.append(CandidTypeVariant{"ok", CandidTypeText{output}});

    std::string s = CandidSerialize(A).as_hex_string();
    if (s !=
        "4449444c016b019cc20171010000ed024f6e63652075706f6e20612074696d652c207468657265207761732061206c6974746c65206769726c206e616d6564204c696c792e20536865206c6f76656420746f20706c6179206f75747369646520696e207468652073756e7368696e652e204f6e65206461792c20736865207361772061206269672c207265642062616c6c20696e2074686520736b792e20497420776173207468652073756e21205368652074686f756768742069742077617320736f207072657474792e0a4c696c792077616e74656420746f20706c61792077697468207468652062616c6c2c206275742069742077617320746f6f206869676820757020696e2074686520736b792e2053686520747269656420746f206a756d7020616e642072656163682069742c206275742073686520636f756c646e27742e205468656e2c207368652068616420616e20696465612e2053686520776f756c6420757365206120737469636b20746f206b6e6f636b20746865")
      IC_API::trap(std::string(__func__) + ": 4c");
  }

  { // Verify serialization & deserialization of CandidTypeVecVariant (unit variants)
    // didc encode '(vec { variant { AdminQuery }; variant { AdminUpdate }; variant { AdminQuery } })'
    // Note: We test unit variants where the variant options have no associated data

    { // serialize
      CandidSerializeTypeTableRegistry::get_instance().clear();

      // Create a template variant with the valid options
      CandidTypeVariant v_template;
      v_template.append("AdminQuery", CandidTypeNull{});
      v_template.append("AdminUpdate", CandidTypeNull{});

      // Vector of selected labels
      std::vector<std::string> labels = {"AdminQuery", "AdminUpdate",
                                         "AdminQuery"};

      CandidArgs A;
      A.append(CandidTypeVecVariant{v_template, labels});

      std::string s = CandidSerialize(A).as_hex_string();
      // Verify it starts with DIDL and has expected structure
      if (s.substr(0, 8) != "4449444c")
        IC_API::trap(std::string(__func__) + ": vec_variant 1a - bad magic");
      // The hex should encode: vec of 3 variants with indices 0, 1, 0
      // Type table + vec length (3) + values (0, 1, 0)
    }

    { // deserialize
      CandidSerializeTypeTableRegistry::get_instance().clear();

      // Create a template variant with the valid options
      CandidTypeVariant v_template;
      v_template.append("AdminQuery", CandidTypeNull{});
      v_template.append("AdminUpdate", CandidTypeNull{});

      // Vector to receive the decoded labels
      std::vector<std::string> labels;

      CandidArgs A;
      A.append(CandidTypeVecVariant{&v_template, &labels});

      // First serialize to get the hex string
      CandidSerializeTypeTableRegistry::get_instance().clear();
      CandidTypeVariant v_template_encode;
      v_template_encode.append("AdminQuery", CandidTypeNull{});
      v_template_encode.append("AdminUpdate", CandidTypeNull{});
      std::vector<std::string> labels_encode = {"AdminQuery", "AdminUpdate",
                                                "AdminQuery"};
      CandidArgs A_encode;
      A_encode.append(CandidTypeVecVariant{v_template_encode, labels_encode});
      std::string hex = CandidSerialize(A_encode).as_hex_string();

      // Now deserialize
      CandidSerializeTypeTableRegistry::get_instance().clear();
      CandidDeserialize(hex, A);

      if (labels.size() != 3)
        IC_API::trap(std::string(__func__) + ": vec_variant 1b - wrong size");
      if (labels[0] != "AdminQuery")
        IC_API::trap(std::string(__func__) + ": vec_variant 1c - labels[0]");
      if (labels[1] != "AdminUpdate")
        IC_API::trap(std::string(__func__) + ": vec_variant 1d - labels[1]");
      if (labels[2] != "AdminQuery")
        IC_API::trap(std::string(__func__) + ": vec_variant 1e - labels[2]");
    }
  }

  { // A variant's fields go on the wire sorted by id (hash), whatever the
    // append order. Hash order is noul, score, choice: append in another order.
    // Verified with an independent decoder (icp-py-core's icp_candid.decode):
    // '(vec { variant { choice }; variant { noul }; variant { score } })'
    CandidSerializeTypeTableRegistry::get_instance().clear();

    CandidTypeVariant v_template;
    v_template.append("choice", CandidTypeNull{});
    v_template.append("score", CandidTypeNull{});
    v_template.append("noul", CandidTypeNull{});

    std::vector<std::string> labels = {"choice", "noul", "score"};

    CandidArgs A;
    A.append(CandidTypeVecVariant{v_template, labels});

    // Identical type tables share one index: the template's variant appears
    // once (before 7.0.0rc4 it appeared twice). didc decodes both the same.
    std::string s = CandidSerialize(A).as_hex_string();
    if (s !=
        "4449444c036b03d8b1a8c8047fd2e6e5c6077fe1febe850c7f6b006d0001020302"
        "0001")
      IC_API::trap(std::string(__func__) + ": vec_variant 2");
  }

  { // An implicit variant field id is the previously appended id + 1, even
    // though the fields are kept sorted by id
    CandidSerializeTypeTableRegistry::get_instance().clear();

    CandidTypeVariant v;
    v.append(CandidTypeNull{});
    if (v.get_field_ids() != std::vector<uint32_t>{0})
      IC_API::trap(std::string(__func__) + ": variant implicit id 1");

    v.append(uint32_t(100), CandidTypeNull{});
    v.append(uint32_t(5), CandidTypeNull{});
    v.append(CandidTypeNull{});
    if (v.get_field_ids() != std::vector<uint32_t>{0, 5, 6, 100})
      IC_API::trap(std::string(__func__) + ": variant implicit id 2");
  }

  // Verify CandidTypePrincipal (https://internetcomputer.org/docs/current/references/id-encoding-spec#decode)
  {
    CandidSerializeTypeTableRegistry::get_instance().clear();
    std::string s = "2ibo7-dia";
    CandidTypePrincipal p = CandidTypePrincipal(s);
    std::vector<std::byte> v = p.get_M().vec();
    std::vector<std::byte> v_expected{std::byte('\x01'), std::byte('\x01'),
                                      std::byte('\x00')};
    if (v != v_expected) IC_API::trap(std::string(__func__) + ": 5a");
  }
  {
    CandidSerializeTypeTableRegistry::get_instance().clear();
    std::string s = "w3gef-eqbai";
    CandidTypePrincipal p = CandidTypePrincipal(s);
    std::vector<std::byte> v = p.get_M().vec();
    std::vector<std::byte> v_expected{std::byte('\x01'), std::byte('\x02'),
                                      std::byte('\x01'), std::byte('\x02')};
    if (v != v_expected) IC_API::trap(std::string(__func__) + ": 5b");
  }

  { // Record width subtyping: an additional field on the wire, of any type,
    // is decoded and discarded, also inside a variant, an opt and a vec
    test_record_width_subtyping();
  }

  { // A vec of variants
    test_vec_variant();
  }

  { // A record field that may be absent on the wire
    test_append_optional();
  }

  { // A type table index that needs 2 bytes
    test_type_table_index_64();
  }

  { // A vec of any element type
    test_vec_of_decode();
  }

  { // An additional field of a vec of records is read and discarded
    test_skip_vec_of();
  }

  return 0;
}