// The expected types of the Candid fuzz targets, picked by an input's first
// byte: fuzz_decoder decodes at them, fuzz_encoder builds values of them.

#pragma once

#include "candid_type_graph.h"

#include <vector>

struct FuzzExpected {
  CandidTypeGraph g;
  std::vector<int> args;
};

inline std::vector<FuzzExpected> make_fuzz_expected() {
  auto prim = [](CandidKind k) { return CandidTypeGraph::prim(k); };
  std::vector<FuzzExpected> out;
  int nat = prim(CandidKind::Nat), text = prim(CandidKind::Text);

  { // Skip everything
    FuzzExpected e;
    int r = prim(CandidKind::Reserved);
    e.args = {r, r, r};
    out.push_back(e);
  }
  { // Primitives, and coercions to them
    FuzzExpected e;
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
    FuzzExpected e;
    int v =
        e.g.variant({{0, prim(CandidKind::Null)}, {1, prim(CandidKind::Int)}});
    int r = e.g.record({{97, nat}, {98, e.g.opt(text)}, {99, e.g.vec(v)}});
    e.args = {r, e.g.vec(r), e.g.opt(e.g.record({}))};
    out.push_back(e);
  }
  { // A recursive type: type List = opt record { head : int; tail : List }
    FuzzExpected e;
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
    FuzzExpected e;
    int f = e.g.func({text}, {nat}, {CANDID_FUNC_QUERY});
    int m = e.g.func({nat}, {}, {});
    int s = e.g.service({{"get", f}, {"set", m}});
    e.args = {f, s, e.g.opt(f)};
    out.push_back(e);
  }
  { // Nested opts, and types with no values
    FuzzExpected e;
    e.args = {e.g.opt(e.g.opt(e.g.opt(nat))), e.g.vec(e.g.opt(e.g.record({}))),
              prim(CandidKind::Null), e.g.opt(prim(CandidKind::Empty))};
    out.push_back(e);
  }
  { // Fixed-width numbers in a variant
    FuzzExpected e;
    e.args = {e.g.variant({{0, prim(CandidKind::Float64)},
                           {1, prim(CandidKind::Float32)},
                           {2, prim(CandidKind::Int64)},
                           {3, prim(CandidKind::Nat32)},
                           {4, prim(CandidKind::Int8)}})};
    out.push_back(e);
  }
  { // More recursive types:
    //   type Tree = variant { leaf : int; node : vec Tree }
    //   type F = func (F) -> (F)
    //   type S = service { get : () -> (S) query }
    FuzzExpected e;
    int tree = e.g.reserve();
    CandidGraphNode t;
    t.kind = CandidKind::Variant;
    t.fields = {{0, prim(CandidKind::Int)}, {1, e.g.vec(tree)}};
    e.g.define(tree, t);
    int f = e.g.reserve();
    CandidGraphNode fn;
    fn.kind = CandidKind::Func;
    fn.args = {f};
    fn.rets = {f};
    e.g.define(f, fn);
    int s = e.g.reserve();
    CandidGraphNode get;
    get.kind = CandidKind::Func;
    get.rets = {s};
    get.modes = {CANDID_FUNC_QUERY};
    CandidGraphNode sn;
    sn.kind = CandidKind::Service;
    sn.methods = {{"get", e.g.add(get)}};
    e.g.define(s, sn);
    e.args = {tree, f, s, e.g.vec(tree)};
    out.push_back(e);
  }
  return out;
}
