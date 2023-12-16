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

  // ---------------------------------------------------------------------------
  // Verify the data
  IC_API::trap("ADD ASSERTS FOR request - " + std::string(__func__));
  if (request.headers.size() == 0)
    IC_API::trap("ASSERT ERROR no 'headers' found - " + std::string(__func__));

  // ---------------------------------------------------------------------------

  IC_HttpResponse response;
  // TODO: set the values for response
  // response.status_code = 200;
  // response.headers = ...;
  // response.body = ...;
  // response.upgrade = ...;
  ic_api.to_wire(response);
}