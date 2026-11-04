// The Candid specification's test suite, run on the IC

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
