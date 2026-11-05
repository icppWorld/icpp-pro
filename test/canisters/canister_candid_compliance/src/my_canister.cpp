// The Candid specification's test suite, run on the IC, and methods that
// send and receive a recursive type

#include "my_canister.h"

#include <string>
#include <vector>

#include "compliance_runner.h"
#include "ic_api.h"

void suites() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  ic_api.from_wire();
  std::vector<std::string> paths;
  for (const auto &s : compliance_suites())
    paths.push_back(s.path);
  ic_api.to_wire(CandidTypeVecText{paths});
}

void run_suite() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  std::string path;
  ic_api.from_wire(CandidTypeText{&path});

  for (const auto &s : compliance_suites()) {
    if (path != s.path) continue;
    ComplianceRunner r(s.path);
    s.run(r);
    std::string failures;
    for (const auto &f : r.failures())
      failures += f + "\n";
    CandidTypeRecord r_out;
    r_out.append("passed", CandidTypeNat64{uint64_t(r.passed())});
    r_out.append("total", CandidTypeNat64{uint64_t(r.total())});
    r_out.append("failures", CandidTypeText{failures});
    ic_api.to_wire(r_out);
    return;
  }
  IC_API::trap("No suite named " + path);
}

// type List = opt record { head : int; tail : List }
static const uint32_t HEAD = CandidTypeGraph::idl_hash("head");
static const uint32_t TAIL = CandidTypeGraph::idl_hash("tail");

static int list_type(CandidTypeGraph &g) {
  int list = g.reserve();
  CandidGraphNode n;
  n.kind = CandidKind::Opt;
  n.inner =
      g.record({{HEAD, CandidTypeGraph::prim(CandidKind::Int)}, {TAIL, list}});
  g.define(list, n);
  return list;
}

// The List 1, 2, ..., n, built as a CandidValue and sent as a CandidTypeValue
void list() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  __uint128_t n{0};
  ic_api.from_wire(CandidTypeNat{&n});

  CandidValue v; // null
  v.kind = CandidKind::Opt;
  for (__uint128_t k = n; k >= 1; --k) {
    CandidValue head;
    head.kind = CandidKind::Int;
    head.n = CandidBigInt::from_u128(k);
    CandidValue node;
    node.kind = CandidKind::Record;
    node.ids = {HEAD, TAIL}; // sorted by id
    node.items = {head, v};
    CandidValue opt;
    opt.kind = CandidKind::Opt;
    opt.items = {node};
    v = opt;
  }
  CandidTypeGraph g;
  int t = list_type(g);
  ic_api.to_wire(CandidTypeValue{g, t, v});
}

// The sum of the heads of a List, received as a CandidValue
void list_sum() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  CandidTypeGraph g;
  int t = list_type(g);
  CandidValue v;
  ic_api.from_wire(CandidTypeValue{g, t, &v});

  __int128_t sum{0};
  for (const CandidValue *p = &v; !p->items.empty();
       p = p->items[0].field(TAIL)) {
    __int128_t head{0};
    if (!p->items[0].field(HEAD)->n.to_i128(head) ||
        __builtin_add_overflow(sum, head, &sum))
      IC_API::trap("list_sum: the sum does not fit in 128 bits");
  }
  ic_api.to_wire(CandidTypeInt{sum});
}
