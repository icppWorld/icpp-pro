// The observability helpers of IC_Management: fetch_canister_logs,
// canister_metrics, subnet_info, subnet_metrics and node_metrics_history.

#include "my_canister.h"

#include <optional>
#include <string>
#include <vector>

#include "ic_api.h"
#include "test_helpers.h"

// '(canister_id, filter, start, end)' -> '("<idx>:<content in hex>|...")';
// filter is "none", "by_idx" or "by_timestamp_nanos". The content is in hex,
// so a record can hold any byte, also the delimiters
IC_Async logs_child_async() {
  IC_API ic_api(CanisterUpdate{"logs_child"}, false);
  std::string canister_id, filter_kind;
  uint64_t start{0}, end{0};
  CandidArgs args;
  args.append(CandidTypePrincipal{&canister_id});
  args.append(CandidTypeText{&filter_kind});
  args.append(CandidTypeNat64{&start});
  args.append(CandidTypeNat64{&end});
  ic_api.from_wire(args);
  std::optional<IC_LogFilter> filter;
  if (filter_kind != "none") {
    filter = IC_LogFilter{filter_kind == "by_timestamp_nanos"
                              ? IC_LogFilter::Kind::ByTimestampNanos
                              : IC_LogFilter::Kind::ByIdx,
                          start, end};
  }
  IC_CallResult<std::vector<IC_CanisterLogRecord>> r =
      co_await IC_Management::fetch_canister_logs(
          CandidTypePrincipal{canister_id}, filter);
  if (!r) {
    ic_api.to_wire(CandidTypeText{error_text(r.error())});
    co_return;
  }
  std::string text;
  for (size_t i = 0; i < r->size(); ++i) {
    const IC_CanisterLogRecord &record = (*r)[i];
    text += (i ? "|" : "") + std::to_string(record.idx) + ":" +
            hex_text(record.content);
  }
  ic_api.to_wire(CandidTypeText{text});
}
void logs_child() { logs_child_async(); }

// '(canister_id)' -> '("memory=..; ...; burned_cycles=..")'
IC_Async metrics_child_async() {
  IC_API ic_api(CanisterUpdate{"metrics_child"}, false);
  std::string canister_id;
  ic_api.from_wire(CandidTypePrincipal{&canister_id});
  IC_CallResult<IC_CanisterMetrics> r =
      co_await IC_Management::canister_metrics(
          CandidTypePrincipal{canister_id});
  if (!r) {
    ic_api.to_wire(CandidTypeText{error_text(r.error())});
    co_return;
  }
  const IC_CyclesConsumed &c = r->cycles_consumed;
  ic_api.to_wire(
      CandidTypeText{"memory=" + nat_text(c.memory) +
                     "; compute_allocation=" + nat_text(c.compute_allocation) +
                     "; ingress_induction=" + nat_text(c.ingress_induction) +
                     "; instructions=" + nat_text(c.instructions) +
                     "; request_and_response_transmission=" +
                     nat_text(c.request_and_response_transmission) +
                     "; uninstall=" + nat_text(c.uninstall) +
                     "; canister_creation=" + nat_text(c.canister_creation) +
                     "; http_outcalls=" + nat_text(c.http_outcalls) +
                     "; burned_cycles=" + nat_text(c.burned_cycles)});
}
void metrics_child() { metrics_child_async(); }

// The subnet of a principal argument, or of this canister when it is
// "aaaaa-aa"
CandidTypePrincipal subnet_from_wire(IC_API &ic_api, uint64_t *start) {
  std::string subnet;
  CandidArgs args;
  args.append(CandidTypePrincipal{&subnet});
  if (start) args.append(CandidTypeNat64{start});
  ic_api.from_wire(args);
  return subnet == "aaaaa-aa" ? ic_api.get_subnet_self()
                              : CandidTypePrincipal{subnet};
}

// '(subnet_id)' -> '("replica_version=..; registry_version=..")'
IC_Async subnet_info_of_async() {
  IC_API ic_api(CanisterUpdate{"subnet_info_of"}, false);
  CandidTypePrincipal subnet = subnet_from_wire(ic_api, nullptr);
  IC_CallResult<IC_SubnetInfo> r = co_await IC_Management::subnet_info(subnet);
  ic_api.to_wire(CandidTypeText{
      r ? "replica_version=" + r->replica_version +
              "; registry_version=" + std::to_string(r->registry_version)
        : error_text(r.error())});
}
void subnet_info_of() { subnet_info_of_async(); }

// '(subnet_id)' -> '("block_height=..; ...")'
IC_Async subnet_metrics_of_async() {
  IC_API ic_api(CanisterUpdate{"subnet_metrics_of"}, false);
  CandidTypePrincipal subnet = subnet_from_wire(ic_api, nullptr);
  IC_CallResult<IC_SubnetMetrics> r =
      co_await IC_Management::subnet_metrics(subnet);
  if (!r) {
    ic_api.to_wire(CandidTypeText{error_text(r.error())});
    co_return;
  }
  ic_api.to_wire(CandidTypeText{
      "block_height=" + nat_text(r->block_height) +
      "; num_canisters=" + nat_text(r->num_canisters) +
      "; canister_state_bytes=" + nat_text(r->canister_state_bytes) +
      "; consumed_cycles_total=" + nat_text(r->consumed_cycles_total) +
      "; update_transactions_total=" + nat_text(r->update_transactions_total) +
      "; million_round_instructions_total=" +
      nat_text(r->million_round_instructions_total)});
}
void subnet_metrics_of() { subnet_metrics_of_async(); }

// '(subnet_id, start_at_timestamp_nanos)'
//   -> '("<timestamp>:<node>/<proposed>/<failures>,...|...")'
IC_Async node_metrics_of_async() {
  IC_API ic_api(CanisterUpdate{"node_metrics_of"}, false);
  uint64_t start{0};
  CandidTypePrincipal subnet = subnet_from_wire(ic_api, &start);
  IC_CallResult<std::vector<IC_NodeMetricsHistoryEntry>> r =
      co_await IC_Management::node_metrics_history(subnet, start);
  if (!r) {
    ic_api.to_wire(CandidTypeText{error_text(r.error())});
    co_return;
  }
  std::string text;
  for (size_t i = 0; i < r->size(); ++i) {
    const IC_NodeMetricsHistoryEntry &e = (*r)[i];
    text += (i ? "|" : "") + std::to_string(e.timestamp_nanos) + ":";
    for (size_t j = 0; j < e.node_metrics.size(); ++j) {
      const IC_NodeMetrics &n = e.node_metrics[j];
      text += (j ? "," : "") + n.node_id + "/" +
              std::to_string(n.num_blocks_proposed_total) + "/" +
              std::to_string(n.num_block_failures_total);
    }
  }
  ic_api.to_wire(CandidTypeText{text});
}
void node_metrics_of() { node_metrics_of_async(); }
