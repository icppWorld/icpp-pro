// Tests for CandidTypeVecOf: a vec of any element type. Each method decodes
// its argument with CandidTypeVecOf and replies with it, encoded with
// CandidTypeVecOf.

#include "test_vec_of.h"
#include "my_canister.h"

#include <string>
#include <vector>

CandidType vec_of_item_type(VecOfItem &item) {
  CandidTypeRecord inner;
  inner.append("q", CandidTypeText(&item.q));
  CandidTypeVariant kind{&item.kind};
  kind.append("a", CandidTypeNull{});
  kind.append("b", CandidTypeNat(&item.amount));
  CandidTypeRecord r;
  r.append("id", CandidTypeNat64(&item.id));
  r.append("note", CandidTypeOptText(&item.note));
  r.append("kind", kind);
  r.append("tags", CandidTypeVecText(&item.tags));
  r.append("inner", inner);
  return r;
}

CandidType vec_of_item_value(const VecOfItem &item) {
  CandidTypeRecord inner;
  inner.append("q", CandidTypeText(item.q));
  // Every case is appended, so that all elements have the same type; the
  // label selects the case that is encoded
  CandidTypeVariant kind{item.kind};
  kind.append("a", CandidTypeNull{});
  kind.append("b", CandidTypeNat(item.amount));
  CandidTypeRecord r;
  r.append("id", CandidTypeNat64(item.id));
  r.append("note", CandidTypeOptText(item.note));
  r.append("kind", kind);
  r.append("tags", CandidTypeVecText(item.tags));
  r.append("inner", inner);
  return r;
}

namespace {
CandidType vec_of_items(const std::vector<VecOfItem> &items) {
  std::vector<CandidType> elements;
  for (const auto &item : items)
    elements.push_back(vec_of_item_value(item));
  VecOfItem prototype;
  prototype.kind = "a";
  return CandidTypeVecOf(vec_of_item_value(prototype), elements);
}

// variant { i32: int32; f64: float64; v128: nat; t: text; u }
struct Global {
  std::string kind;
  int32_t i32{0};
  double f64{0};
  __uint128_t v128{0};
  std::string t;
};

CandidType global_value(const Global &g) {
  CandidTypeVariant v{g.kind};
  v.append("i32", CandidTypeInt32(g.i32));
  v.append("f64", CandidTypeFloat64(g.f64));
  v.append("v128", CandidTypeNat(g.v128));
  v.append("t", CandidTypeText(g.t));
  v.append("u", CandidTypeNull{});
  return v;
}

// record { id: principal; n: nat64; m: opt nat }
struct NodeItem {
  std::string id;
  uint64_t n{0};
  std::optional<__uint128_t> m;
};
// record { ts: nat64; items: vec NodeItem }
struct NodeEntry {
  uint64_t ts{0};
  std::vector<NodeItem> items;
};

CandidType node_item_value(const NodeItem &it) {
  CandidTypeRecord r;
  r.append("id", CandidTypePrincipal(it.id.empty() ? "aaaaa-aa" : it.id));
  r.append("n", CandidTypeNat64(it.n));
  r.append("m", CandidTypeOptNat(it.m));
  return r;
}

CandidType node_entry_value(const NodeEntry &e) {
  std::vector<CandidType> items;
  for (const auto &it : e.items)
    items.push_back(node_item_value(it));
  CandidTypeRecord r;
  r.append("ts", CandidTypeNat64(e.ts));
  r.append("items", CandidTypeVecOf(node_item_value(NodeItem{}), items));
  return r;
}

// record { k: opt nat }
CandidType k_value(const std::optional<__uint128_t> &k) {
  CandidTypeRecord r;
  r.append("k", CandidTypeOptNat(k));
  return r;
}

CandidType vec_of_k(const std::vector<std::optional<__uint128_t>> &ks) {
  std::vector<CandidType> elements;
  for (const auto &k : ks)
    elements.push_back(k_value(k));
  return CandidTypeVecOf(k_value(std::nullopt), elements);
}

CandidType vec_of_k_type(std::optional<__uint128_t> &k,
                         std::vector<std::optional<__uint128_t>> &ks) {
  return CandidTypeVecOf(
      [&]() -> CandidType {
        k.reset();
        CandidTypeRecord r;
        r.append("k", CandidTypeOptNat(&k));
        return r;
      },
      [&]() { ks.push_back(k); });
}
} // namespace

void roundtrip_vec_of_items() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  VecOfItem item;
  std::vector<VecOfItem> items;
  ic_api.from_wire(CandidTypeVecOf(
      [&]() -> CandidType {
        item = {};
        return vec_of_item_type(item);
      },
      [&]() { items.push_back(item); }));
  ic_api.to_wire(vec_of_items(items));
}

void roundtrip_vec_of_variants() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  Global g;
  std::vector<Global> gs;
  ic_api.from_wire(CandidTypeVecOf(
      [&]() -> CandidType {
        g = {};
        CandidTypeVariant v{&g.kind};
        v.append("i32", CandidTypeInt32(&g.i32));
        v.append("f64", CandidTypeFloat64(&g.f64));
        v.append("v128", CandidTypeNat(&g.v128));
        v.append("t", CandidTypeText(&g.t));
        v.append("u", CandidTypeNull{});
        return v;
      },
      [&]() { gs.push_back(g); }));
  std::vector<CandidType> elements;
  for (const auto &x : gs)
    elements.push_back(global_value(x));
  ic_api.to_wire(CandidTypeVecOf(global_value(Global{"u"}), elements));
}

void roundtrip_vec_of_nested() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  NodeItem it;
  NodeEntry e;
  std::vector<NodeEntry> es;
  ic_api.from_wire(CandidTypeVecOf(
      [&]() -> CandidType {
        e = {};
        CandidTypeRecord r;
        r.append("ts", CandidTypeNat64(&e.ts));
        r.append("items", CandidTypeVecOf(
                              [&]() -> CandidType {
                                it = {};
                                CandidTypeRecord ri;
                                ri.append("id", CandidTypePrincipal(&it.id));
                                ri.append("n", CandidTypeNat64(&it.n));
                                ri.append("m", CandidTypeOptNat(&it.m));
                                return ri;
                              },
                              [&]() { e.items.push_back(it); }));
        return r;
      },
      [&]() { es.push_back(e); }));
  std::vector<CandidType> elements;
  for (const auto &x : es)
    elements.push_back(node_entry_value(x));
  ic_api.to_wire(CandidTypeVecOf(node_entry_value(NodeEntry{}), elements));
}

// Two fields with a vec of the same element type share one type table
void roundtrip_vec_of_shared() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  std::optional<__uint128_t> k1, k2;
  std::vector<std::optional<__uint128_t>> r1, r2;
  CandidTypeRecord r_in;
  r_in.append("r1", vec_of_k_type(k1, r1));
  r_in.append("r2", vec_of_k_type(k2, r2));
  ic_api.from_wire(r_in);
  CandidTypeRecord r_out;
  r_out.append("r1", vec_of_k(r1));
  r_out.append("r2", vec_of_k(r2));
  ic_api.to_wire(r_out);
}

// An element of another type than the element type traps
void vec_of_wrong_element() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  ic_api.to_wire(CandidTypeVecOf(CandidTypeNat64(uint64_t(1)),
                                 {CandidTypeText("not a nat64")}));
}
