// The lifecycle helpers of IC_Management: update_settings, start, stop and
// delete a canister, deposit_cycles, and the full canister_status.

#include "my_canister.h"

#include <string>
#include <vector>

#include "ic_api.h"
#include "test_helpers.h"

namespace {
std::string visibility_text(const IC_Visibility &v) {
  switch (v.kind) {
  case IC_Visibility::Kind::Public:
    return "public";
  case IC_Visibility::Kind::AllowedViewers: {
    std::string s = "allowed_viewers(";
    for (size_t i = 0; i < v.allowed_viewers.size(); ++i) {
      s += (i ? "," : "") + v.allowed_viewers[i];
    }
    return s + ")";
  }
  case IC_Visibility::Kind::Controllers:
    break;
  }
  return "controllers";
}

// The settings of a test case
IC_CanisterSettings settings_for(const std::string &name,
                                 const std::string &self,
                                 const std::string &caller) {
  IC_CanisterSettings s;
  if (name == "controllers") {
    s.controllers = std::vector<std::string>{self, caller};
  } else if (name == "only_caller") {
    s.controllers = std::vector<std::string>{caller};
  } else if (name == "public_logs") {
    s.log_visibility = IC_Visibility{IC_Visibility::Kind::Public, {}};
  } else if (name == "viewers") {
    s.log_visibility =
        IC_Visibility{IC_Visibility::Kind::AllowedViewers, {caller}};
    s.status_visibility =
        IC_Visibility{IC_Visibility::Kind::AllowedViewers, {caller}};
    s.snapshot_visibility = IC_Visibility{IC_Visibility::Kind::Public, {}};
  } else if (name == "limits") {
    s.freezing_threshold = 2'592'000;
    s.reserved_cycles_limit = 5'000'000'000'000;
    s.wasm_memory_limit = 3'221'225'472;
    s.wasm_memory_threshold = 1'073'741'824;
    s.log_memory_limit = 4096;
  } else if (name == "env") {
    s.environment_variables =
        std::vector<IC_EnvironmentVariable>{{"A", "1"}, {"B", "two"}};
  } else if (name == "all") {
    s.controllers = std::vector<std::string>{self};
    s.compute_allocation = 1;
    s.memory_allocation = 2;
    s.freezing_threshold = 3;
    s.reserved_cycles_limit = 4;
    s.wasm_memory_limit = 5;
    s.minimum_incoming_canister_call_cycles = 6;
    s.log_visibility = IC_Visibility{IC_Visibility::Kind::Public, {}};
    s.log_memory_limit = 7;
    s.snapshot_visibility =
        IC_Visibility{IC_Visibility::Kind::AllowedViewers, {caller}};
    s.status_visibility = IC_Visibility{IC_Visibility::Kind::Controllers, {}};
    s.wasm_memory_threshold = 8;
    s.environment_variables = std::vector<IC_EnvironmentVariable>{{"A", "1"}};
  }
  return s;
}

// The reply of a call with an empty reply: "ok", or the error
void reply_ok_or_error(IC_API &ic_api, const IC_CallResult<void> &r) {
  ic_api.to_wire(CandidTypeText{r ? std::string("ok") : error_text(r.error())});
}

// '(canister_id)', read in the method's IC_API
std::string canister_id_from_wire(IC_API &ic_api) {
  std::string canister_id;
  ic_api.from_wire(CandidTypePrincipal{&canister_id});
  return canister_id;
}
} // namespace

// '(canister_id, case)' -> '("ok")' or '("rej:...")'
IC_Async update_settings_async() {
  IC_API ic_api(CanisterUpdate{"update_settings"}, false);
  std::string canister_id, name;
  CandidArgs args;
  args.append(CandidTypePrincipal{&canister_id});
  args.append(CandidTypeText{&name});
  ic_api.from_wire(args);
  IC_CanisterSettings settings =
      settings_for(name, ic_api.get_canister_self().get_text(),
                   ic_api.get_caller().get_text());
  IC_CallResult<void> r = co_await IC_Management::update_settings(
      CandidTypePrincipal{canister_id}, settings);
  reply_ok_or_error(ic_api, r);
}
void update_settings() { update_settings_async(); }

// '(canister_id)' -> '("ok")' or '("rej:...")'
IC_Async start_child_async() {
  IC_API ic_api(CanisterUpdate{"start_child"}, false);
  std::string canister_id = canister_id_from_wire(ic_api);
  IC_CallResult<void> r =
      co_await IC_Management::start_canister(CandidTypePrincipal{canister_id});
  reply_ok_or_error(ic_api, r);
}
void start_child() { start_child_async(); }

IC_Async stop_child_async() {
  IC_API ic_api(CanisterUpdate{"stop_child"}, false);
  std::string canister_id = canister_id_from_wire(ic_api);
  IC_CallResult<void> r =
      co_await IC_Management::stop_canister(CandidTypePrincipal{canister_id});
  reply_ok_or_error(ic_api, r);
}
void stop_child() { stop_child_async(); }

IC_Async delete_child_async() {
  IC_API ic_api(CanisterUpdate{"delete_child"}, false);
  std::string canister_id = canister_id_from_wire(ic_api);
  IC_CallResult<void> r =
      co_await IC_Management::delete_canister(CandidTypePrincipal{canister_id});
  reply_ok_or_error(ic_api, r);
}
void delete_child() { delete_child_async(); }

// '(canister_id, cycles)' -> '("ok")' or '("rej:...")'
IC_Async deposit_child_async() {
  IC_API ic_api(CanisterUpdate{"deposit_child"}, false);
  std::string canister_id;
  __uint128_t cycles{0};
  CandidArgs args;
  args.append(CandidTypePrincipal{&canister_id});
  args.append(CandidTypeNat{&cycles});
  ic_api.from_wire(args);
  IC_CallResult<void> r = co_await IC_Management::deposit_cycles(
      CandidTypePrincipal{canister_id}, cycles);
  reply_ok_or_error(ic_api, r);
}
void deposit_child() { deposit_child_async(); }

// '(canister_id)' -> '("<every field of canister_status>")'
IC_Async canister_status_full_async() {
  IC_API ic_api(CanisterUpdate{"canister_status_full"}, false);
  std::string canister_id = canister_id_from_wire(ic_api);
  IC_CallResult<IC_CanisterStatus> r =
      co_await IC_Management::canister_status(CandidTypePrincipal{canister_id});
  if (!r) {
    ic_api.to_wire(CandidTypeText{error_text(r.error())});
    co_return;
  }
  const IC_CanisterStatus &s = *r;
  const IC_DefiniteCanisterSettings &st = s.settings;
  std::string text;
  text += std::string("status=") +
          (s.status == IC_API::CanisterStatus::Running    ? "running"
           : s.status == IC_API::CanisterStatus::Stopping ? "stopping"
                                                          : "stopped");
  text += "; ready_for_migration=" +
          std::string(s.ready_for_migration ? "true" : "false");
  text += "; version=" + std::to_string(s.version);
  text += "; controllers=" + std::to_string(st.controllers.size());
  text += "; compute_allocation=" + nat_text(st.compute_allocation);
  text += "; memory_allocation=" + nat_text(st.memory_allocation);
  text += "; freezing_threshold=" + nat_text(st.freezing_threshold);
  text += "; reserved_cycles_limit=" + nat_text(st.reserved_cycles_limit);
  text += "; minimum_incoming_canister_call_cycles=" +
          nat_text(st.minimum_incoming_canister_call_cycles);
  text += "; log_visibility=" + visibility_text(st.log_visibility);
  text += "; log_memory_limit=" + nat_text(st.log_memory_limit);
  text += "; snapshot_visibility=" + visibility_text(st.snapshot_visibility);
  text += "; status_visibility=" + visibility_text(st.status_visibility);
  text += "; wasm_memory_limit=" + nat_text(st.wasm_memory_limit);
  text += "; wasm_memory_threshold=" + nat_text(st.wasm_memory_threshold);
  text += "; environment_variables=";
  for (size_t i = 0; i < st.environment_variables.size(); ++i) {
    text += (i ? "," : "") + st.environment_variables[i].name + "=" +
            st.environment_variables[i].value;
  }
  text += "; module_hash=" + std::string(s.module_hash ? "set" : "none");
  text += "; memory_size=" + nat_text(s.memory_size);
  text += "; wasm_memory_size=" + nat_text(s.memory_metrics.wasm_memory_size);
  text += "; log_memory_store_size=" +
          nat_text(s.memory_metrics.log_memory_store_size);
  text += "; cycles=" + nat_text(s.cycles);
  text += "; reserved_cycles=" + nat_text(s.reserved_cycles);
  text +=
      "; idle_cycles_burned_per_day=" + nat_text(s.idle_cycles_burned_per_day);
  text += "; num_calls_total=" + nat_text(s.query_stats.num_calls_total);
  ic_api.to_wire(CandidTypeText{text});
}
void canister_status_full() { canister_status_full_async(); }

// '(key_name, message_hash)' -> '(public_key, signature)', with secp256r1
IC_Async ecdsa_r1_async() {
  IC_API ic_api(CanisterUpdate{"ecdsa_r1"}, false);
  std::string key_name;
  IC_Blob message_hash;
  CandidArgs args;
  args.append(CandidTypeText{&key_name});
  args.append(CandidTypeVecNat8{&message_hash});
  ic_api.from_wire(args);
  IC_EcdsaKeyId key{IC_API::EcdsaCurve::Secp256r1, key_name};
  std::vector<IC_Blob> path{{'i', 'c', 'p', 'p'}};
  IC_CallResult<IC_PublicKey> public_key =
      co_await IC_Management::ecdsa_public_key(key, path);
  if (!public_key) {
    ic_api.to_wire(CandidTypeText{error_text(public_key.error())});
    co_return;
  }
  IC_CallResult<IC_Blob> signature =
      co_await IC_Management::sign_with_ecdsa(message_hash, path, key);
  ic_api.to_wire(CandidTypeText{
      signature
          ? "public_key=" + std::to_string(public_key->public_key.size()) +
                " bytes; signature=" + std::to_string(signature->size()) +
                " bytes"
          : error_text(signature.error())});
}
void ecdsa_r1() { ecdsa_r1_async(); }
