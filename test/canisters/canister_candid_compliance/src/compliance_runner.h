// Runs the asserts of the Candid specification's test suite.
//
// The suite files in ../suite are turned into C++ (suite_*.cpp) by
// scripts/candid_compliance/generate.py. Each assert becomes one call to
// ComplianceRunner::check, with its expected types as a CandidTypeGraph.
//
// The semantics are those of dfinity/candid's own runner:
//   :   the input decodes at the types
//   !:  it does not
//   ==  both inputs decode, to equal values
//   !=  both inputs decode, to values that are not equal
// A binary input is decoded with CandidDecoder, a textual one is parsed with
// CandidTextValue.

#pragma once

#include "candid_decoder.h"
#include "candid_text_value.h"
#include "candid_type_graph.h"
#include "candid_value.h"

#include <string>
#include <vector>

struct ComplianceInput {
  bool blob{false};
  std::string data; // the bytes of a blob, or the text
};

enum class Op { Decodes, Fails, Equal, NotEqual };

class ComplianceRunner {
public:
  explicit ComplianceRunner(std::string file) : m_file(std::move(file)) {}

  void check(int line, const std::string &desc, const CandidTypeGraph &g,
             const std::vector<int> &args, Op op, const ComplianceInput &left,
             const ComplianceInput &right);

  size_t passed() const { return m_passed; }
  size_t total() const { return m_total; }
  // One line per failed assert: file:line description: reason
  const std::vector<std::string> &failures() const { return m_failures; }

private:
  std::string m_file;
  size_t m_passed{0};
  size_t m_total{0};
  std::vector<std::string> m_failures;
};

struct ComplianceSuite {
  const char *path;
  void (*run)(ComplianceRunner &r);
};

// Every suite, in suites.cpp (generated)
const std::vector<ComplianceSuite> &compliance_suites();

// Builders for the generated code
CandidGraphNode cn_opt(int inner);
CandidGraphNode cn_vec(int inner);
CandidGraphNode cn_record(std::vector<CandidGraphField> fields);
CandidGraphNode cn_variant(std::vector<CandidGraphField> fields);
CandidGraphNode cn_func(std::vector<int> args, std::vector<int> rets,
                        std::vector<uint8_t> modes);
CandidGraphNode cn_service(std::vector<CandidGraphMethod> methods);
