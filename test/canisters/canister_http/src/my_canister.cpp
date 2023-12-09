#include "my_canister.h"

#include <algorithm>
#include <memory>
#include <string>
#include <variant>

#include "ic0.h"
#include "ic_api.h"

#include <json/json.hpp>

//----------------------------------------------------------------------------------
// Run all roundtrip tests

void roundtrip_record_vec_record() {
  // This represents the headers in the http_request of the IC
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);

  // ---------------------------------------------------------------------------
  // Get the data from the wire
  // '(record { headers = vec { record {name = "H1N" : text; value = "H1V"}; record {name = "H2N" : text; value = "H2V"}; record {name = "H3N" : text; value = "H3V"}; } })'

  // Define vectors to store the data from the vec-of-records
  std::vector<std::string> names;
  std::vector<std::string> values;

  // Pass in a record-of-vecs using the correct keys, and icpp will know what to do ;-)
  CandidTypeRecord header_fields_in;
  header_fields_in.append("name", CandidTypeVecText{&names});
  header_fields_in.append("value", CandidTypeVecText{&values});
  CandidTypeVecRecord headers_in{&header_fields_in};

  // The HttpRequest
  CandidTypeRecord http_in;
  http_in.append("headers", headers_in);

  ic_api.from_wire(http_in);

  // ---------------------------------------------------------------------------
  // Verify the data
  if (names.size() == 0)
    IC_API::trap("ASSERT ERROR no 'names' found - " + std::string(__func__));
  if (names.size() != values.size())
    IC_API::trap(
        "ASSERT ERROR different size found for 'names' & 'values' vectors - " +
        std::string(__func__));
  if (names[0] != "H1N")
    IC_API::trap("ASSERT ERROR names[0] - " + std::string(__func__));
  if (names[1] != "H2N")
    IC_API::trap("ASSERT ERROR names[1] - " + std::string(__func__));
  if (names[2] != "H3N")
    IC_API::trap("ASSERT ERROR names[2] - " + std::string(__func__));
  if (values[0] != "H1V")
    IC_API::trap("ASSERT ERROR values[0] - " + std::string(__func__));
  if (values[1] != "H2V")
    IC_API::trap("ASSERT ERROR values[1] - " + std::string(__func__));
  if (values[2] != "H3V")
    IC_API::trap("ASSERT ERROR values[2] - " + std::string(__func__));
  // ---------------------------------------------------------------------------

  CandidTypeRecord header_fields_out;
  header_fields_out.append("name", CandidTypeVecText{&names});
  header_fields_out.append("value", CandidTypeVecText{&values});
  CandidTypeVecRecord headers_out{header_fields_out};

  // The HttpResponse
  CandidTypeRecord http_out;
  http_out.append("headers", headers_out);

  ic_api.to_wire(http_out);
}