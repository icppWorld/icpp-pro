// Runs the asserts of the Candid specification's test suite.

#include "compliance_runner.h"

#include <set>
#include <utility>

static bool parse_input(const ComplianceInput &in, const CandidTypeGraph &g,
                        const std::vector<int> &args,
                        std::vector<CandidValue> &out, std::string &err) {
  if (in.blob)
    return CandidDecoder::decode(
        reinterpret_cast<const uint8_t *>(in.data.data()), in.data.size(), g,
        args, out, err);
  return CandidTextValue::parse(in.data, g, args, out, err);
}

void ComplianceRunner::check(int line, const std::string &desc,
                             const CandidTypeGraph &g,
                             const std::vector<int> &args, Op op,
                             const ComplianceInput &left,
                             const ComplianceInput &right) {
  ++m_total;
  std::vector<CandidValue> lv;
  std::string err;
  bool lok = parse_input(left, g, args, lv, err);
  std::string why;

  switch (op) {
  case Op::Decodes:
    if (!lok) why = "does not decode: " + err;
    break;
  case Op::Fails:
    if (lok) why = "decodes, to " + candid_values_to_text(lv);
    break;
  case Op::Equal:
  case Op::NotEqual: {
    if (!lok) {
      why = "left side does not decode: " + err;
      break;
    }
    std::vector<CandidValue> rv;
    if (!parse_input(right, g, args, rv, err)) {
      why = "right side does not decode: " + err;
      break;
    }
    bool equal = lv == rv;
    if (equal != (op == Op::Equal))
      why = std::string(equal ? "equal" : "not equal") + ": " +
            candid_values_to_text(lv) + " vs " + candid_values_to_text(rv);
    break;
  }
  }

  record(line, desc, why);
}

void ComplianceRunner::record(int line, const std::string &desc,
                              const std::string &why) {
  if (why.empty()) {
    ++m_passed;
  } else {
    m_failures.push_back(m_file + ":" + std::to_string(line) + " " + desc +
                         ": " + why);
  }
}

// Two types are the same: equal kinds, fields, methods and annotations, all
// the way down. Coinductive, for recursive types.
static bool same_type(const CandidTypeGraph &g1, int t1,
                      const CandidTypeGraph &g2, int t2,
                      std::set<std::pair<int, int>> &seen) {
  if (!seen.insert({t1, t2}).second) return true;
  CandidKind k = g1.kind(t1);
  if (k != g2.kind(t2)) return false;
  if (!CandidTypeGraph::is_node(t1) || !CandidTypeGraph::is_node(t2))
    return k != CandidKind::Future;
  const CandidGraphNode &a = g1.node(t1);
  const CandidGraphNode &b = g2.node(t2);
  auto same_list = [&](const std::vector<int> &x, const std::vector<int> &y) {
    if (x.size() != y.size()) return false;
    for (size_t i = 0; i < x.size(); ++i)
      if (!same_type(g1, x[i], g2, y[i], seen)) return false;
    return true;
  };
  switch (k) {
  case CandidKind::Opt:
  case CandidKind::Vec:
    return same_type(g1, a.inner, g2, b.inner, seen);
  case CandidKind::Record:
  case CandidKind::Variant:
    if (a.fields.size() != b.fields.size()) return false;
    for (size_t i = 0; i < a.fields.size(); ++i)
      if (a.fields[i].id != b.fields[i].id ||
          !same_type(g1, a.fields[i].type, g2, b.fields[i].type, seen))
        return false;
    return true;
  case CandidKind::Func:
    return a.modes == b.modes && same_list(a.args, b.args) &&
           same_list(a.rets, b.rets);
  case CandidKind::Service:
    if (a.methods.size() != b.methods.size()) return false;
    for (size_t i = 0; i < a.methods.size(); ++i)
      if (a.methods[i].name != b.methods[i].name ||
          !same_type(g1, a.methods[i].type, g2, b.methods[i].type, seen))
        return false;
    return true;
  default:
    return false;
  }
}

static std::string encode_why(const CandidTypeGraph &g,
                              const std::vector<int> &args,
                              const ComplianceInput &text,
                              const std::string &ours,
                              const std::string &reference) {
  const auto *po = reinterpret_cast<const uint8_t *>(ours.data());
  const auto *pr = reinterpret_cast<const uint8_t *>(reference.data());
  std::string err;

  CandidTypeGraph wo, wr;
  std::vector<int> ao, ar;
  if (!CandidDecoder::read_types(po, ours.size(), wo, ao, err))
    return "the encoding is not valid Candid: " + err;
  if (!CandidDecoder::read_types(pr, reference.size(), wr, ar, err))
    return "the reference is not valid Candid: " + err;
  if (ao.size() != ar.size())
    return "the encoding has " + std::to_string(ao.size()) +
           " arguments, the reference " + std::to_string(ar.size());
  std::set<std::pair<int, int>> seen;
  for (size_t i = 0; i < ao.size(); ++i)
    if (!same_type(wo, ao[i], wr, ar[i], seen))
      return "argument " + std::to_string(i) + " has the type " +
             wo.to_string(ao[i]) + ", the reference " + wr.to_string(ar[i]);

  std::vector<CandidValue> vo, vr, vt;
  if (!CandidDecoder::decode(po, ours.size(), g, args, vo, err))
    return "the encoding does not decode: " + err;
  if (!CandidDecoder::decode(pr, reference.size(), g, args, vr, err))
    return "the reference does not decode: " + err;
  if (!CandidTextValue::parse(text.data, g, args, vt, err))
    return "the value does not parse: " + err;
  if (!(vo == vr) || !(vo == vt))
    return "the encoding decodes to " + candid_values_to_text(vo) +
           ", the reference to " + candid_values_to_text(vr);
  return "";
}

void ComplianceRunner::check_encode(int line, const std::string &desc,
                                    const CandidTypeGraph &g,
                                    const std::vector<int> &args,
                                    const ComplianceInput &text,
                                    const std::function<VecBytes()> &encode,
                                    const ComplianceInput &reference) {
  ++m_total;
  CandidSerializeTypeTableRegistry::get_instance().clear();
  VecBytes B = encode();
  std::string ours;
  for (std::byte b : B.vec())
    ours.push_back(static_cast<char>(b));
  CandidSerializeTypeTableRegistry::get_instance().clear();
  record(line, desc, encode_why(g, args, text, ours, reference.data));
}

CandidGraphNode cn_opt(int inner) {
  CandidGraphNode n;
  n.kind = CandidKind::Opt;
  n.inner = inner;
  return n;
}

CandidGraphNode cn_vec(int inner) {
  CandidGraphNode n;
  n.kind = CandidKind::Vec;
  n.inner = inner;
  return n;
}

CandidGraphNode cn_record(std::vector<CandidGraphField> fields) {
  CandidGraphNode n;
  n.kind = CandidKind::Record;
  n.fields = std::move(fields);
  return n;
}

CandidGraphNode cn_variant(std::vector<CandidGraphField> fields) {
  CandidGraphNode n;
  n.kind = CandidKind::Variant;
  n.fields = std::move(fields);
  return n;
}

CandidGraphNode cn_func(std::vector<int> args, std::vector<int> rets,
                        std::vector<uint8_t> modes) {
  CandidGraphNode n;
  n.kind = CandidKind::Func;
  n.args = std::move(args);
  n.rets = std::move(rets);
  n.modes = std::move(modes);
  return n;
}

CandidGraphNode cn_service(std::vector<CandidGraphMethod> methods) {
  CandidGraphNode n;
  n.kind = CandidKind::Service;
  n.methods = std::move(methods);
  return n;
}
