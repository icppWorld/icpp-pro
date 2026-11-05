// Unit tests for icpp-candid's CandidEncoder.
//
// The expected bytes are those of didc 0.6.2, `didc encode -t '<types>'`, or
// with `-d <file>` for named types, and of candid's Rust tests (serde.rs).

#include "unit_test_candid_encoder.h"

#include <string>
#include <vector>

#include "candid_bigint.h"
#include "candid_decoder.h"
#include "candid_encoder.h"
#include "candid_text_value.h"
#include "ic_api.h"

namespace {

void check(bool ok, const std::string &what) {
  if (!ok) IC_API::trap("unit_test_candid_encoder: " + what);
}

std::string hex(const std::vector<uint8_t> &b) {
  static const char *digits = "0123456789abcdef";
  std::string s;
  for (uint8_t c : b) {
    s.push_back(digits[c >> 4]);
    s.push_back(digits[c & 15]);
  }
  return s;
}

std::vector<uint8_t> unhex(const std::string &s) {
  std::vector<uint8_t> b;
  for (size_t i = 0; i + 1 < s.size(); i += 2)
    b.push_back(uint8_t(std::stoi(s.substr(i, 2), nullptr, 16)));
  return b;
}

// Encodes the text value at the argument types, compares with the expected
// bytes, and decodes them back to the same values
void check_encode(const std::string &what, const CandidTypeGraph &g,
                  const std::vector<int> &args, const std::string &text,
                  const std::string &expected) {
  std::vector<CandidValue> values, back;
  std::string err;
  check(CandidTextValue::parse(text, g, args, values, err),
        what + ": the value parses: " + err);
  std::vector<uint8_t> out;
  check(CandidEncoder::encode(g, args, values, out, err),
        what + ": encodes: " + err);
  check(hex(out) == expected, what + ": " + hex(out) + " != " + expected);
  check(CandidDecoder::decode(out.data(), out.size(), g, args, back, err),
        what + ": decodes: " + err);
  check(back == values, what + ": round trip");
}

// The value does not fit the type: the encoder says why, and writes nothing
void check_refused(const std::string &what, const CandidTypeGraph &g, int t,
                   const CandidValue &v) {
  std::vector<uint8_t> out{0xaa};
  std::string err;
  check(!CandidEncoder::encode_value(g, t, v, out, err) && !err.empty(),
        what + ": refused");
  check(out.size() == 1, what + ": nothing written");
}

int prim(CandidKind k) { return CandidTypeGraph::prim(k); }

uint32_t h(const char *name) { return CandidTypeGraph::idl_hash(name); }

__attribute__((noinline)) void test_leb128() {
  auto leb = [](const char *dec) {
    CandidBigInt n;
    CandidBigInt::parse(dec, n);
    std::vector<uint8_t> out;
    n.to_leb128(out);
    return hex(out);
  };
  auto sleb = [](const char *dec) {
    CandidBigInt n;
    CandidBigInt::parse(dec, n);
    std::vector<uint8_t> out;
    n.to_sleb128(out);
    return hex(out);
  };
  check(leb("0") == "00", "leb 0");
  check(leb("127") == "7f", "leb 127");
  check(leb("128") == "8001", "leb 128");
  check(leb("624485") == "e58e26", "leb 624485");
  check(leb("4294967296") == "8080808010", "leb 2^32");
  check(sleb("0") == "00", "sleb 0");
  check(sleb("-1") == "7f", "sleb -1");
  check(sleb("63") == "3f", "sleb 63");
  check(sleb("64") == "c000", "sleb 64");
  check(sleb("-64") == "40", "sleb -64");
  check(sleb("-65") == "bf7f", "sleb -65");
  check(sleb("-123456") == "c0bb78", "sleb -123456");
  check(sleb("-4294967296") == "8080808070", "sleb -2^32");
  {
    CandidBigInt n;
    CandidBigInt::parse("-1", n);
    std::vector<uint8_t> out;
    check(!n.to_leb128(out) && out.empty(), "leb of a negative is refused");
  }

  // Large values: the groups read back to the same number
  for (const char *dec : {"340282366920938463463374607431768211456",
                          "-340282366920938463463374607431768211457",
                          "123456789012345678901234567890123456789012345",
                          "-9223372036854775808", "18446744073709551615"}) {
    CandidBigInt n;
    CandidBigInt::parse(dec, n);
    std::vector<uint8_t> out;
    n.to_sleb128(out);
    std::vector<uint8_t> groups;
    for (uint8_t b : out)
      groups.push_back(b & 0x7f);
    check(CandidBigInt::from_sleb_groups(groups) == n,
          std::string("sleb round trip ") + dec);
    if (!n.is_negative()) {
      out.clear();
      n.to_leb128(out);
      groups.clear();
      for (uint8_t b : out)
        groups.push_back(b & 0x7f);
      check(CandidBigInt::from_leb_groups(groups) == n,
            std::string("leb round trip ") + dec);
    }
  }
}

// type List = record { head : int; tail : opt List }
int make_list(CandidTypeGraph &g) {
  int list = g.reserve();
  int tail = g.opt(list);
  CandidGraphNode n;
  n.kind = CandidKind::Record;
  n.fields = {{h("head"), prim(CandidKind::Int)}, {h("tail"), tail}};
  if (n.fields[0].id > n.fields[1].id) std::swap(n.fields[0], n.fields[1]);
  g.define(list, n);
  return list;
}

__attribute__((noinline)) void test_recursive() {
  {
    CandidTypeGraph g;
    int list = make_list(g);
    check_encode("List", g, {list},
                 "(record {head=42; tail=opt record{head=43; tail=null}})",
                 "4449444c026c02a0d2aca8047c90eddae704016e0001002a012b00");
    // An inline opt List is the same type as the field's opt List
    check_encode("(List, opt List)", g, {list, g.opt(list)},
                 "(record {head=42; tail=null}, null)",
                 "4449444c026c02a0d2aca8047c90eddae704016e000200012a0000");
  }
  {
    // type List = opt record { head : int; tail : List }
    CandidTypeGraph g;
    int list = g.reserve();
    int rec = g.record({{h("head"), prim(CandidKind::Int)}, {h("tail"), list}});
    CandidGraphNode n;
    n.kind = CandidKind::Opt;
    n.inner = rec;
    g.define(list, n);
    check_encode("opt List", g, {list},
                 "(opt record{head=1; tail=opt record{head=2; tail=null}})",
                 "4449444c026e016c02a0d2aca8047c90eddae7040001000101010200");

    // Nesting is limited as in the decoder: 2 levels per element
    auto list_of = [](int n) {
      std::string s = "null";
      for (int i = n; i > 0; --i)
        s = "opt record{head=" + std::to_string(i) + "; tail=" + s + "}";
      return "(" + s + ")";
    };
    std::vector<CandidValue> values;
    std::vector<uint8_t> out;
    std::string err;
    check(CandidTextValue::parse(list_of(63), g, {list}, values, err) &&
              CandidEncoder::encode(g, {list}, values, out, err),
          "a List of 63 elements encodes: " + err);
    std::vector<CandidValue> back;
    check(CandidDecoder::decode(out.data(), out.size(), g, {list}, back, err) &&
              back == values,
          "a List of 63 elements decodes: " + err);
    // One more element, in front: opt record {head=0; tail=<the 63>}
    // (head hashes below tail, so it is the first field)
    CandidValue head, elem, opt;
    head.kind = CandidKind::Int;
    elem.kind = CandidKind::Record;
    elem.ids = {h("head"), h("tail")};
    elem.items = {head, values[0]};
    opt.kind = CandidKind::Opt;
    opt.items = {elem};
    out.clear();
    err.clear();
    check(!CandidEncoder::encode(g, {list}, {opt}, out, err) &&
              err.find("nest") != std::string::npos,
          "a List of 64 elements nests too deep: " + err);
  }
  {
    // Rust's test_equivalent_types: each struct is a named type
    //   type A = record { typeb: opt B };
    //   type B = record { typea: opt A };
    //   RootType = record { typeas: vec A }
    CandidTypeGraph g;
    int root = g.reserve(), a = g.reserve(), b = g.reserve();
    int vec_a = g.vec(a), opt_b = g.opt(b), opt_a = g.opt(a);
    CandidGraphNode n;
    n.kind = CandidKind::Record;
    n.fields = {{h("typeas"), vec_a}};
    g.define(root, n);
    n.fields = {{h("typeb"), opt_b}};
    g.define(a, n);
    n.fields = {{h("typea"), opt_a}};
    g.define(b, n);
    check_encode("RootType", g, {root}, "(record {typeas = vec {}})",
                 "4449444c066c01acd4dbb905016d026c01e8e0add601036e046c01e7e0add"
                 "601056e02010000");
    check_encode("TypeB", g, {b}, "(record {typea = null})",
                 "4449444c046c01e7e0add601016e026c01e8e0add601036e00010000");
  }
  {
    // A recursive type read from the wire has no named nodes: the cycle is
    // broken where it returns, and the table comes out as it went in
    const std::string m =
        "4449444c026c02a0d2aca8047c90eddae704016e0001002a012b00";
    std::vector<uint8_t> bytes = unhex(m);
    CandidTypeGraph w;
    std::vector<int> args;
    std::string err;
    check(CandidDecoder::read_types(bytes.data(), bytes.size(), w, args, err),
          "read the List header: " + err);
    std::vector<uint8_t> out;
    check(CandidEncoder::encode_header(w, args, out, err), "re-encode: " + err);
    check(hex(out) == m.substr(0, 2 * out.size()),
          "a wire List header re-encodes as it was: " + hex(out));
  }
}

__attribute__((noinline)) void test_tables() {
  {
    CandidTypeGraph g;
    auto rec_a = [&] { return g.record({{h("a"), prim(CandidKind::Int)}}); };
    check_encode("equal inline types share an entry", g,
                 {rec_a(), rec_a(), g.opt(rec_a())},
                 "(record{a=1}, record{a=2}, null)",
                 "4449444c026c01617c6e0003000001010200");
  }
  {
    CandidTypeGraph g;
    int inner = g.record({{h("a"), prim(CandidKind::Int)}});
    int rec = g.record(
        {{h("a"), prim(CandidKind::Int)}, {h("b"), g.vec(inner)}});
    check_encode("an entry is numbered before its children", g, {g.vec(rec)},
                 "(vec {record{a=1; b=vec{record{a=2}}}})",
                 "4449444c046d016c02617c62026d036c01617c010001010102");
  }
  {
    CandidTypeGraph g;
    int r = g.record({{h("a"), prim(CandidKind::Nat)}});
    int f = g.func({r}, {g.record({{h("a"), prim(CandidKind::Nat)}})}, {1});
    check_encode("func", g, {f}, "(func \"aaaaa-aa\".\"m\")",
                 "4449444c026a0101010101016c01617d0100010100016d");
  }
  {
    CandidTypeGraph g;
    int fa = g.func({prim(CandidKind::Text)}, {prim(CandidKind::Nat)}, {1});
    int fb = g.func({prim(CandidKind::Nat)}, {}, {});
    int s = g.service({{"b", fb}, {"a", fa}});
    int v = g.variant(
        {{h("x"), prim(CandidKind::Null)}, {h("y"), prim(CandidKind::Text)}});
    check_encode("service, vec int8, variant, int, nat", g,
                 {s, g.vec(prim(CandidKind::Int8)), v, prim(CandidKind::Int),
                  prim(CandidKind::Nat)},
                 "(service \"aaaaa-aa\", vec {1;-2}, variant {y=\"h\xc3\xa9\"}, "
                 "-123456, 624485)",
                 "4449444c0569020161010162026a0171017d01016a017d00006d776b0278"
                 "7f7971050003047c7d01000201fe010368c3a9c0bb78e58e26");
  }
  {
    CandidTypeGraph g;
    int rec = g.record(
        {{0, g.opt(prim(CandidKind::Int))}, {1, prim(CandidKind::Text)}});
    check_encode("vec null, reserved, floats, principal, tuple", g,
                 {g.vec(prim(CandidKind::Null)), prim(CandidKind::Reserved),
                  prim(CandidKind::Float32), prim(CandidKind::Float64),
                  prim(CandidKind::Principal), rec},
                 "(vec {null;null}, null, 1.5, -2.25, principal \"2vxsx-fae\", "
                 "record {opt 7; \"x\"})",
                 "4449444c036d7f6c02000201716e7c06007073726801020000c03f000000"
                 "00000002c001010401070178");
  }
}

__attribute__((noinline)) void test_value_checks() {
  CandidTypeGraph g;
  auto value = [](CandidKind k) {
    CandidValue v;
    v.kind = k;
    return v;
  };

  CandidValue v = value(CandidKind::Nat);
  CandidBigInt::parse("-1", v.n);
  check_refused("a negative nat", g, prim(CandidKind::Nat), v);

  v = value(CandidKind::Nat8);
  v.u = 256;
  check_refused("nat8 256", g, prim(CandidKind::Nat8), v);
  v = value(CandidKind::Int16);
  v.i = -32769;
  check_refused("int16 -32769", g, prim(CandidKind::Int16), v);

  v = value(CandidKind::Text);
  v.text = "\xff";
  check_refused("text that is not UTF-8", g, prim(CandidKind::Text), v);

  v = value(CandidKind::Principal);
  v.bytes.assign(30, 1);
  check_refused("a principal of 30 bytes", g, prim(CandidKind::Principal), v);

  check_refused("a value of type empty", g, prim(CandidKind::Empty),
                value(CandidKind::Empty));
  check_refused("a bool at nat", g, prim(CandidKind::Nat),
                value(CandidKind::Bool));

  int opt_nat = g.opt(prim(CandidKind::Nat));
  v = value(CandidKind::Opt);
  v.items.assign(2, value(CandidKind::Nat));
  check_refused("an opt of two values", g, opt_nat, v);

  int vec_nat16 = g.vec(prim(CandidKind::Nat16));
  v = value(CandidKind::Vec);
  v.packed = true;
  v.elem_kind = CandidKind::Nat8;
  v.bytes = {1, 2};
  check_refused("packed nat8 at vec nat16", g, vec_nat16, v);
  v.elem_kind = CandidKind::Nat16;
  v.bytes = {1, 2, 3};
  check_refused("packed nat16 of 3 bytes", g, vec_nat16, v);
  int vec_bool = g.vec(prim(CandidKind::Bool));
  v.elem_kind = CandidKind::Bool;
  v.bytes = {2};
  check_refused("packed bool 2", g, vec_bool, v);

  v = value(CandidKind::Vec);
  v.repeat = 3;
  v.items = {value(CandidKind::Nat)};
  check_refused("a repeat at vec nat", g, g.vec(prim(CandidKind::Nat)), v);

  int rec = g.record({{1, prim(CandidKind::Nat)}, {2, prim(CandidKind::Nat)}});
  v = value(CandidKind::Record);
  v.ids = {1};
  v.items = {value(CandidKind::Nat)};
  check_refused("a record without a field", g, rec, v);
  v.ids = {1, 3};
  v.items = {value(CandidKind::Nat), value(CandidKind::Nat)};
  check_refused("a record with another field", g, rec, v);
  v.ids = {1, 2};
  v.items[1].absent = true;
  check_refused("an absent field of type nat", g, rec, v);

  int var = g.variant({{1, prim(CandidKind::Null)}});
  v = value(CandidKind::Variant);
  v.ids = {2};
  v.items = {value(CandidKind::Null)};
  check_refused("a variant case the type lacks", g, var, v);

  // An absent field at an opt is written as null; reserved takes any value
  int rec2 = g.record({{1, opt_nat}, {2, prim(CandidKind::Reserved)}});
  v = value(CandidKind::Record);
  v.ids = {1, 2};
  v.items = {value(CandidKind::Opt), value(CandidKind::Text)};
  v.items[0].absent = true;
  std::vector<uint8_t> out;
  std::string err;
  check(CandidEncoder::encode_value(g, rec2, v, out, err) && hex(out) == "00",
        "an absent opt field is null, reserved takes any value: " + err);

  // A repeat of a zero-width element: the length, no bytes
  int vec_null = g.vec(prim(CandidKind::Null));
  v = value(CandidKind::Vec);
  v.repeat = 1000000;
  v.items = {value(CandidKind::Null)};
  out.clear();
  check(CandidEncoder::encode_value(g, vec_null, v, out, err) &&
            hex(out) == "c0843d",
        "vec null of 1e6, repeated: " + err);
  v.repeat = 1000001;
  check_refused("vec null of 1e6 + 1, over the decoder's budget", g, vec_null,
                v);

  // The argument count must match the values
  std::vector<CandidValue> none;
  out.clear();
  check(!CandidEncoder::encode(g, {prim(CandidKind::Nat)}, none, out, err),
        "one type and no values is refused");
}

} // namespace

int unit_test_candid_encoder() {
  test_leb128();
  test_recursive();
  test_tables();
  test_value_checks();
  return 0;
}
