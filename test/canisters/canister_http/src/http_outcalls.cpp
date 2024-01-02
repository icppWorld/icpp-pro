// Demo of http outcalls, so the canister can call external Web2.0 HTTP servers
#include "canister.h"
#include "http_outcalls.h"

#include <algorithm>
#include <memory>
#include <string>
#include <variant>

#include "ic_api.h"

#include <json/json.hpp>

/*
 Reference: https://internetcomputer.org/docs/current/developer-docs/integrations/https-outcalls/https-outcalls-get

 To get today's ICP-USD exchange rate of May 1, 2023, with timestamp=1682978460:
 $ curl 'https://api.pro.coinbase.com/products/ICP-USD/candles?start=1682978460&end=1682978460&granularity=60'
 [[1682978460,5.714,5.718,5.714,5.714,243.5678]]
   timestamp ,low  ,high ,open ,close,volume

*/
void get_icp_usd_exchange() {
  // update call, because HTTP outcalls go through consensus
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);

  // Setup the URL and its query parameters
  uint64_t start_timestamp = 1682978460; //May 1, 2023 22:01:00 GMT
  uint64_t seconds_of_time = 60;         //start with 60 seconds
  std::string host = "api.pro.coinbase.com";

  std::string url = "https://" + host + "/products/ICP-USD/candles" +
                    "?start=" + std::to_string(start_timestamp) +
                    "&end=" + std::to_string(start_timestamp) +
                    "&granularity=" + std::to_string(seconds_of_time);

  // NOTE: http outcalls are under development...
  //       this method is a placeholder.

  std::string response{"TODO"};
  ic_api.to_wire(CandidTypeText(response));
}
