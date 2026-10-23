// The outcall and provisional helpers of IC_Management:
// flexible_http_request, http_request with pay-as-you-go pricing, and the
// provisional methods of a local network.

#include "my_canister.h"

#include <optional>
#include <string>
#include <vector>

#include "ic_api.h"
#include "test_helpers.h"

namespace {
std::string usage_text(const IC_HttpUsage &u) {
  if (!u.reported) return "-";
  return u.exceeded ? "exceeded" : nat_text(u.used);
}

std::string global_text(IC_FlexibleHttpResult::GlobalError e) {
  using G = IC_FlexibleHttpResult::GlobalError;
  switch (e) {
  case G::Timeout:
    return "timeout";
  case G::OutOfCycles:
    return "out_of_cycles";
  case G::ResponsesTooLarge:
    return "responses_too_large";
  case G::TooManyRejects:
    return "too_many_rejects";
  case G::None:
    break;
  }
  return "none";
}
} // namespace

// '(url, min, max, total, with_replication)'
//   -> '("ok:<status>,<status>,..")' or '("err:<global>:<nodes>:<message>")'
IC_Async flexible_get_async() {
  IC_API ic_api(CanisterUpdate{"flexible_get"}, false);
  std::string url;
  uint32_t min{0}, max{0}, total{0};
  bool with_replication{false};
  CandidArgs args;
  args.append(CandidTypeText{&url});
  args.append(CandidTypeNat32{&min});
  args.append(CandidTypeNat32{&max});
  args.append(CandidTypeNat32{&total});
  args.append(CandidTypeBool{&with_replication});
  ic_api.from_wire(args);
  IC_FlexibleHttpRequest request;
  request.request.url = url;
  request.request.max_response_bytes = 10'000;
  if (with_replication)
    request.replication = IC_API::HttpOutcallReplicationCounts{total, min, max};
  IC_CallResult<IC_FlexibleHttpResult> r =
      co_await IC_Management::flexible_http_request(request);
  if (!r) {
    ic_api.to_wire(CandidTypeText{error_text(r.error())});
    co_return;
  }
  std::string text;
  if (r->ok) {
    text = "ok:";
    for (size_t i = 0; i < r->responses.size(); ++i) {
      text += (i ? "," : "") + nat_text(r->responses[i].status);
    }
  } else {
    text = "err:" + global_text(r->global_error) + ":";
    for (size_t i = 0; i < r->node_details.size(); ++i) {
      const IC_HttpNodeDetail &d = r->node_details[i];
      text += (i ? "," : "") + d.node_id + "/" +
              usage_text(d.raw_response_bytes) + "/" +
              usage_text(d.http_roundtrip_time_ms) + "/" +
              usage_text(d.transform_instructions) + "/" +
              usage_text(d.transformed_response_bytes) + "/" +
              usage_text(d.cycles) + "/" +
              (d.error_code ? *d.error_code + "=" + *d.error_message : "-");
    }
    text += ":" + r->message;
  }
  ic_api.to_wire(CandidTypeText{text});
}
void flexible_get() { flexible_get_async(); }

// '(url, is_replicated)' -> '("status=..; bytes=..")': with pricing_version 2
IC_Async http_get_v2_async() {
  IC_API ic_api(CanisterUpdate{"http_get_v2"}, false);
  std::string url;
  bool is_replicated{true};
  CandidArgs args;
  args.append(CandidTypeText{&url});
  args.append(CandidTypeBool{&is_replicated});
  ic_api.from_wire(args);
  IC_HttpOutcallRequest request;
  request.url = url;
  request.max_response_bytes = 10'000;
  request.is_replicated = is_replicated;
  request.pricing_version = 2;
  IC_CallResult<IC_HttpOutcallResponse> r =
      co_await IC_Management::http_request(request);
  ic_api.to_wire(
      CandidTypeText{r ? "status=" + nat_text(r->status) +
                             "; bytes=" + std::to_string(r->body.size())
                       : error_text(r.error())});
}
void http_get_v2() { http_get_v2_async(); }

// '(amount, specified_id)' -> '("<principal>")' or '("rej:...")'; amount and
// specified_id are vecs of 0 or 1 element
IC_Async provisional_create_async() {
  IC_API ic_api(CanisterUpdate{"provisional_create"}, false);
  std::vector<__uint128_t> amount;
  std::vector<std::string> specified_id;
  CandidArgs args;
  args.append(CandidTypeVecNat{&amount});
  args.append(CandidTypeVecPrincipal{&specified_id});
  ic_api.from_wire(args);
  std::optional<__uint128_t> amount_opt;
  if (!amount.empty()) amount_opt = amount[0];
  std::optional<CandidTypePrincipal> id;
  if (!specified_id.empty()) id = CandidTypePrincipal{specified_id[0]};
  IC_CallResult<CandidTypePrincipal> r =
      co_await IC_Management::provisional_create_canister_with_cycles(
          amount_opt, {}, id);
  ic_api.to_wire(CandidTypeText{r ? r->get_text() : error_text(r.error())});
}
void provisional_create() { provisional_create_async(); }

// '(canister_id, amount)' -> '("ok")' or '("rej:...")'
IC_Async provisional_top_up_async() {
  IC_API ic_api(CanisterUpdate{"provisional_top_up"}, false);
  std::string canister_id;
  __uint128_t amount{0};
  CandidArgs args;
  args.append(CandidTypePrincipal{&canister_id});
  args.append(CandidTypeNat{&amount});
  ic_api.from_wire(args);
  IC_CallResult<void> r = co_await IC_Management::provisional_top_up_canister(
      CandidTypePrincipal{canister_id}, amount);
  ic_api.to_wire(CandidTypeText{r ? std::string("ok") : error_text(r.error())});
}
void provisional_top_up() { provisional_top_up_async(); }
