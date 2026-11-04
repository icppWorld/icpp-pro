// Runs the asserts of the Candid specification's test suite.

#include "compliance_runner.h"

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

  if (why.empty()) {
    ++m_passed;
  } else {
    m_failures.push_back(m_file + ":" + std::to_string(line) + " " + desc +
                         ": " + why);
  }
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
