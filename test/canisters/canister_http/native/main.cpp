// Main entry point for a native debug executable.
// Build it with: `icpp build-native` from the parent folder where 'icpp.toml' resides

#include "main.h"

#include <iostream>

#include "../src/my_canister.h"

// The Mock IC
#include "icpp_hooks.h"
#include "mock_ic.h"

int main() {
  bool exit_on_fail = true;
  MockIC mockIC(exit_on_fail);

  // The pretend principals of the caller
  std::string my_principal{
      "expmt-gtxsw-inftj-ttabj-qhp5s-nozup-n3bbo-k7zvn-dg4he-knac3-lae"};
  std::string anonymous_principal{"2vxsx-fae"};

  bool silent_on_trap = true;

  //----------------------------------------------------------------------------------
  // Run all roundtrip tests

  // ------------------------------------------------------------------------
  // To call the canister with curl:
  // MAINNET
  //  curl -X GET -H "Content-Type: application/json" -d '{"key1":"value1", "key2":"value2"}' https://$(dfx canister --ic id my_canister).raw.icp0.io/my-route
  //  or
  //  curl -X GET -H "Content-Type: application/json" -d '{"key1":"value1", "key2":"value2"}'  https://<canister-id>.raw.icp0.io/my-route
  //
  // To skip the asserts, use this:
  //  curl -X GET -H "skip-asserts: yes" -H "Content-Type: application/json" -d '{"key1":"value1", "key2":"value2"}' https://$(dfx canister --ic id my_canister).raw.icp0.io/my-route
  //  or
  //  curl -X GET -H "skip-asserts: yes" -H "Content-Type: application/json" -d '{"key1":"value1", "key2":"value2"}'  https://<canister-id>.raw.icp0.io/my-route

  // LOCAL
  //  curl -X GET -H "skip-asserts: yes" -H "Content-Type: application/json" -d '{"key1":"value1", "key2":"value2"}' http://localhost:$(dfx info webserver-port)/?canisterId=$(dfx canister id my_canister)
  //
  // The http_request method is called with this candid:
  //
  // '(record { url = "/my-route" : text; method = "" : text; body = blob "{\22key1\22:\22value1\22, \22key2\22:\22value2\22}"; headers = vec { record { "host"; "5ugrv-zqaaa-aaaag-acfna-cai.raw.icp0.io" };record { "x-real-ip"; "21.98.241.245" }; record { "x-forwarded-for"; "21.98.241.245" }; record { "x-forwarded-proto"; "https" }; record { "x-request-id"; "d9b41db4-4e97-8276-14f1-6db7d007b57d" }; record { "user-agent"; "curl/7.81.0" }; record { "accept"; "*/*" }; }; certificate_version = opt (2 : nat16); })'
  // which is same as:
  //   (
  //     record {
  //         url = "/my-route";
  //         method = "GET";
  //         body = blob "{\22key1\22:\22value1\22, \22key2\22:\22value2\22}";
  //         headers = vec {
  //         record { "host"; "5ugrv-zqaaa-aaaag-acfna-cai.raw.icp0.io" };
  //         record { "x-real-ip"; "21.98.241.245" };
  //         record { "x-forwarded-for"; "21.98.241.245" };
  //         record { "x-forwarded-proto"; "https" };
  //         record { "x-request-id"; "d9b41db4-4e97-8276-14f1-6db7d007b57d" };
  //         record { "user-agent"; "curl/7.81.0" };
  //         record { "accept"; "*/*" };
  //         record { "content-type"; "application/json" };
  //         };
  //         certificate_version = opt (2 : nat16);
  //     },
  //   )
  //   Or, using hashed values:
  //   (
  //     record {
  //         5_843_823 = "/my-route";
  //         156_956_385 = "GET";
  //         1_092_319_906 = blob "{\22key1\22:\22value1\22, \22key2\22:\22value2\22}";
  //         1_661_489_734 = vec {
  //         record { "host"; "5ugrv-zqaaa-aaaag-acfna-cai.raw.icp0.io" };
  //         record { "x-real-ip"; "21.98.241.245" };
  //         record { "x-forwarded-for"; "21.98.241.245" };
  //         record { "x-forwarded-proto"; "https" };
  //         record { "x-request-id"; "d9b41db4-4e97-8276-14f1-6db7d007b57d" };
  //         record { "user-agent"; "curl/7.81.0" };
  //         record { "accept"; "*/*" };
  //         record { "content-type"; "application/json" };
  //         };
  //         1_661_892_784 = opt (2 : nat16);
  //     },
  //   )
  // -> HttpResponse (TODO)
  // Use compact type table of didc
  mockIC.run_test(
      "http_request", http_request,
      "4449444c056c05efd6e40271e1edeb4a71a2f5ed880401c6a4a1980602b0f1b99806046d7b6d036c02007101716e7a0100092f6d792d726f75746503474554227b226b657931223a2276616c756531222c20226b657932223a2276616c756532227d0904686f73742735756772762d7a716161612d61616161672d6163666e612d6361692e7261772e696370302e696f09782d7265616c2d69700d32342e39362e3234302e3134350f782d666f727761726465642d666f720d32342e39362e3234302e31343511782d666f727761726465642d70726f746f0568747470730c782d726571756573742d69642465643166653437622d643433652d643962312d363264642d6535306436346438633465620e636f6e74656e742d6c656e6774680233340a757365722d6167656e740b6375726c2f372e38312e3006616363657074032a2f2a0c636f6e74656e742d74797065106170706c69636174696f6e2f6a736f6e010200",
      "4449444c116c02000101016d716c006c02007101716d036c02007101716c02007101716c02007101716c02007101716c02007101716c02007101716c02007101716c02007101716c02007101716c04a2f5ed88040fc6a4a19806049ce9c69906109aa1b2f90c7a6d7b6e7e010e227b226b657931223a2276616c756531222c20226b657932223a2276616c756532227d0904686f73742735756772762d7a716161612d61616161672d6163666e612d6361692e7261772e696370302e696f09782d7265616c2d69700d32342e39362e3234302e3134350f782d666f727761726465642d666f720d32342e39362e3234302e31343511782d666f727761726465642d70726f746f0568747470730c782d726571756573742d69642465643166653437622d643433652d643962312d363264642d6535306436346438633465620e636f6e74656e742d6c656e6774680233340a757365722d6167656e740b6375726c2f372e38312e3006616363657074032a2f2a0c636f6e74656e742d74797065106170706c69636174696f6e2f6a736f6e0100c800",
      silent_on_trap, my_principal);

  // returns 1 if any tests failed
  return mockIC.test_summary();
}
