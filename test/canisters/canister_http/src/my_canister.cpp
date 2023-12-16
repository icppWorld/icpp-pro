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

// TODO: http_request_update

void http_request() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);

  // Outcomment this if you want to see the Candid that is sent to the canister.
  // This is how I got the hex string for the test in ../native/main.cpp
  // (-) Deploy the canister to the mainnet
  // (-) Issue the curl request shown in ../native/main.cpp
  // IC_API::debug_print("Entered http_request !");
  // IC_API::trap("Hex String from wire = " + ic_api.get_B_in().as_hex_string());

  IC_HttpRequest request;
  ic_api.from_wire(request);

  bool skip_asserts{false};
  for (IC_HeaderField headerField : request.headers) {
    if (headerField.name == "skip-asserts") {
      if (headerField.value == "yes") {
        skip_asserts = true;
      }
      break;
    }
  }

  // ---------------------------------------------------------------------------
  // Verify the data
  if (!skip_asserts) {
    if (request.method != "GET")
      IC_API::trap("ASSERT ERROR request.method - " + std::string(__func__));
    if (request.url != "/my-route")
      IC_API::trap("ASSERT ERROR request.url - " + std::string(__func__));
    if (request.headers.size() != 9)
      IC_API::trap("ASSERT ERROR size of headers - " + std::string(__func__));
    if (request.headers[0].name != "host" ||
        request.headers[0].value != "5ugrv-zqaaa-aaaag-acfna-cai.raw.icp0.io")
      IC_API::trap("ASSERT ERROR request.headers[0] - " +
                   std::string(__func__));
    if (request.headers[1].name != "x-real-ip" ||
        request.headers[1].value != "24.96.240.145")
      IC_API::trap("ASSERT ERROR request.headers[1] - " +
                   std::string(__func__));
    if (request.headers[2].name != "x-forwarded-for" ||
        request.headers[2].value != "24.96.240.145")
      IC_API::trap("ASSERT ERROR request.headers[2] - " +
                   std::string(__func__));
    if (request.headers[3].name != "x-forwarded-proto" ||
        request.headers[3].value != "https")
      IC_API::trap("ASSERT ERROR request.headers[3] - " +
                   std::string(__func__));
    if (request.headers[4].name != "x-request-id" ||
        request.headers[4].value != "ed1fe47b-d43e-d9b1-62dd-e50d64d8c4eb")
      IC_API::trap("ASSERT ERROR request.headers[4] - " +
                   std::string(__func__));
    if (request.headers[5].name != "content-length" ||
        request.headers[5].value != "34")
      IC_API::trap("ASSERT ERROR request.headers[5] - " +
                   std::string(__func__));
    if (request.headers[6].name != "user-agent" ||
        request.headers[6].value != "curl/7.81.0")
      IC_API::trap("ASSERT ERROR request.headers[6] - " +
                   std::string(__func__));
    if (request.headers[7].name != "accept" ||
        request.headers[7].value != "*/*")
      IC_API::trap("ASSERT ERROR request.headers[7] - " +
                   std::string(__func__));
    if (request.headers[8].name != "content-type" ||
        request.headers[8].value != "application/json")
      IC_API::trap("ASSERT ERROR request.headers[8] - " +
                   std::string(__func__));
  }
  // ---------------------------------------------------------------------------

  IC_HttpResponse response;
  response.status_code = 200;
  response.headers = request.headers;
  response.body = request.body;
  response.upgrade = false;
  ic_api.to_wire(response);
}