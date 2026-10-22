// Typed helpers for the management canister - see ic_management.h.

#include "ic_api.h"

#if defined(__cpp_impl_coroutine)

#include "ic0.h"

namespace {
// The default and maximum response size of an HTTPS outcall
constexpr uint64_t HTTP_MAX_RESPONSE_BYTES = 2'000'000;

CandidTypePrincipal management() { return CandidTypePrincipal{"aaaaa-aa"}; }

IC_CallError sign_cost_error(IC_API::SignCostError e) {
  IC_CallError error;
  error.kind = IC_CallError::Kind::SignCostFailed;
  error.sign_cost_error = e;
  return error;
}

std::optional<std::string>
principal_text(const std::optional<CandidTypePrincipal> &p) {
  if (!p.has_value()) return std::nullopt;
  CandidTypePrincipal copy = *p;
  return copy.get_text();
}

CandidTypeRecord ecdsa_key_id(const IC_EcdsaKeyId &key) {
  CandidTypeRecord key_id;
  key_id.append("curve",
                CandidTypeVariant{key.curve == IC_API::EcdsaCurve::Secp256r1
                                      ? "secp256r1"
                                      : "secp256k1"});
  key_id.append("name", CandidTypeText{key.name});
  return key_id;
}

CandidTypeRecord schnorr_key_id(const IC_SchnorrKeyId &key) {
  CandidTypeRecord key_id;
  key_id.append(
      "algorithm",
      CandidTypeVariant{key.algorithm == IC_API::SchnorrAlgorithm::Ed25519
                            ? "ed25519"
                            : "bip340secp256k1"});
  key_id.append("name", CandidTypeText{key.name});
  return key_id;
}

CandidTypeRecord vetkd_key_id(const IC_VetKdKeyId &key) {
  CandidTypeRecord key_id;
  key_id.append("curve", CandidTypeVariant{"bls12_381_g2"});
  key_id.append("name", CandidTypeText{key.name});
  return key_id;
}

// A call whose reply is a record with one blob field
IC_Task<IC_CallResult<IC_Blob>> call_for_blob(IC_Call call, std::string field) {
  IC_CallResult<IC_CallReply> r = co_await call;
  if (!r) co_return r.error();
  IC_Blob blob;
  CandidTypeRecord result;
  result.append(field, CandidTypeVecNat8{&blob});
  r->decode(result);
  co_return blob;
}

// A call whose reply is record { public_key : blob; chain_code : blob }
IC_Task<IC_CallResult<IC_PublicKey>> call_for_public_key(IC_Call call) {
  IC_CallResult<IC_CallReply> r = co_await call;
  if (!r) co_return r.error();
  IC_PublicKey key;
  CandidTypeRecord result;
  result.append("public_key", CandidTypeVecNat8{&key.public_key});
  result.append("chain_code", CandidTypeVecNat8{&key.chain_code});
  r->decode(result);
  co_return key;
}

// The http_request_result of an HTTPS outcall, also the type of a transform
CandidTypeRecord http_result_type() {
  CandidTypeRecord header;
  header.append("name", CandidTypeVecText{});
  header.append("value", CandidTypeVecText{});
  CandidTypeRecord result;
  result.append("status", CandidTypeNat{});
  result.append("headers", CandidTypeVecRecord{header});
  result.append("body", CandidTypeVecNat8{});
  return result;
}

const char *http_method(IC_HttpMethod method) {
  switch (method) {
  case IC_HttpMethod::Get:
    return "get";
  case IC_HttpMethod::Head:
    return "head";
  case IC_HttpMethod::Post:
    return "post";
  case IC_HttpMethod::Put:
    return "put";
  case IC_HttpMethod::Delete:
    return "delete";
  case IC_HttpMethod::Patch:
    return "patch";
  }
  return "get";
}

// log_visibility, snapshot_visibility, status_visibility
CandidTypeVariant visibility_value(const IC_Visibility &v) {
  switch (v.kind) {
  case IC_Visibility::Kind::Public:
    return CandidTypeVariant{"public"};
  case IC_Visibility::Kind::AllowedViewers:
    return CandidTypeVariant{"allowed_viewers",
                             CandidTypeVecPrincipal{v.allowed_viewers}};
  case IC_Visibility::Kind::Controllers:
    break;
  }
  return CandidTypeVariant{"controllers"};
}

// The type to decode a visibility into; finish with visibility_from
CandidTypeVariant visibility_type(std::string &label,
                                  std::vector<std::string> &viewers) {
  CandidTypeVariant v{&label};
  v.append("controllers", CandidTypeNull{});
  v.append("public", CandidTypeNull{});
  v.append("allowed_viewers", CandidTypeVecPrincipal{&viewers});
  return v;
}

IC_Visibility visibility_from(const std::string &label,
                              const std::vector<std::string> &viewers) {
  IC_Visibility v;
  if (label == "public") v.kind = IC_Visibility::Kind::Public;
  else if (label == "allowed_viewers") {
    v.kind = IC_Visibility::Kind::AllowedViewers;
    v.allowed_viewers = viewers;
  }
  return v;
}

// The fields of canister_settings that are set; an opt field without a value
// is left out of the record. Returns whether any field is set.
bool append_settings(CandidTypeRecord &record,
                     const IC_CanisterSettings &settings) {
  bool has_settings{false};
  if (settings.controllers.has_value()) {
    record.append("controllers", CandidTypeOptVec{CandidTypeVecPrincipal{
                                     *settings.controllers}});
    has_settings = true;
  }
  auto opt_nat = [&](const char *name, const std::optional<__uint128_t> &v) {
    if (v.has_value()) {
      record.append(name, CandidTypeOptNat{*v});
      has_settings = true;
    }
  };
  auto opt_visibility = [&](const char *name,
                            const std::optional<IC_Visibility> &v) {
    if (v.has_value()) {
      record.append(name, CandidTypeOptVariant{visibility_value(*v)});
      has_settings = true;
    }
  };
  opt_nat("compute_allocation", settings.compute_allocation);
  opt_nat("memory_allocation", settings.memory_allocation);
  opt_nat("freezing_threshold", settings.freezing_threshold);
  opt_nat("reserved_cycles_limit", settings.reserved_cycles_limit);
  opt_nat("wasm_memory_limit", settings.wasm_memory_limit);
  opt_nat("minimum_incoming_canister_call_cycles",
          settings.minimum_incoming_canister_call_cycles);
  opt_visibility("log_visibility", settings.log_visibility);
  opt_nat("log_memory_limit", settings.log_memory_limit);
  opt_visibility("snapshot_visibility", settings.snapshot_visibility);
  opt_visibility("status_visibility", settings.status_visibility);
  opt_nat("wasm_memory_threshold", settings.wasm_memory_threshold);
  if (settings.environment_variables.has_value()) {
    std::vector<std::string> names;
    std::vector<std::string> values;
    for (const auto &env : *settings.environment_variables) {
      names.push_back(env.name);
      values.push_back(env.value);
    }
    CandidTypeRecord env_var;
    env_var.append("name", CandidTypeVecText{names});
    env_var.append("value", CandidTypeVecText{values});
    record.append("environment_variables",
                  CandidTypeOptVec{CandidTypeVecRecord{env_var}});
    has_settings = true;
  }
  return has_settings;
}

// A call with argument record { canister_id } and an empty reply
IC_Task<IC_CallResult<void>>
call_for_canister(const char *method, CandidTypePrincipal canister_id) {
  CandidTypeRecord arg;
  arg.append("canister_id", canister_id);
  IC_CallResult<IC_CallReply> r =
      co_await IC_Call::bounded_wait(management(), method).with_arg(arg);
  if (!r) co_return r.error();
  co_return IC_CallResult<void>{};
}

CandidTypePrincipal canister_self() {
  std::vector<uint8_t> bytes(ic0_canister_self_size());
  ic0_canister_self_copy(reinterpret_cast<uintptr_t>(bytes.data()), 0,
                         (uint32_t)bytes.size());
  return CandidTypePrincipal{bytes};
}
} // namespace

IC_Task<IC_CallResult<IC_Blob>> IC_Management::raw_rand() {
  IC_CallResult<IC_CallReply> r =
      co_await IC_Call::bounded_wait(management(), "raw_rand");
  if (!r) co_return r.error();
  IC_Blob bytes;
  r->decode(CandidTypeVecNat8{&bytes});
  co_return bytes;
}

IC_Task<IC_CallResult<CandidTypePrincipal>>
IC_Management::create_canister(IC_CanisterSettings settings,
                               __uint128_t extra_cycles) {
  CandidTypeRecord settings_record;
  bool has_settings = append_settings(settings_record, settings);

  // An opt field without a value is left out of the record
  CandidTypeRecord arg;
  if (has_settings)
    arg.append("settings", CandidTypeOptRecord{settings_record});
  arg.append("sender_canister_version",
             CandidTypeOptNat64{ic0_canister_version()});

  IC_CallResult<IC_CallReply> r =
      co_await IC_Call::guaranteed_response(management(), "create_canister")
          .with_arg(arg)
          .with_cycles(IC_API::cost_create_canister() + extra_cycles);
  if (!r) co_return r.error();
  std::string canister_id;
  CandidTypeRecord result;
  result.append("canister_id", CandidTypePrincipal{&canister_id});
  r->decode(result);
  co_return CandidTypePrincipal{canister_id};
}

IC_Task<IC_CallResult<IC_CanisterStatus>>
IC_Management::canister_status(CandidTypePrincipal canister_id) {
  CandidTypeRecord arg;
  arg.append("canister_id", canister_id);
  IC_CallResult<IC_CallReply> r =
      co_await IC_Call::bounded_wait(management(), "canister_status")
          .with_arg(arg);
  if (!r) co_return r.error();

  // The reply has more fields; they are skipped (record width subtyping).
  // The fields the IC added after canister_status first shipped may be absent
  // in a reply of an older replica, or of a mock (append_optional).
  IC_CanisterStatus status;
  IC_DefiniteCanisterSettings &st = status.settings;
  std::string label;
  CandidTypeVariant status_variant{&label};
  status_variant.append("running", CandidTypeNull{});
  status_variant.append("stopping", CandidTypeNull{});
  status_variant.append("stopped", CandidTypeNull{});

  std::string log_label, snapshot_label, status_label;
  std::vector<std::string> log_viewers, snapshot_viewers, status_viewers;
  std::vector<std::string> env_names, env_values;
  CandidTypeRecord env_var;
  env_var.append("name", CandidTypeVecText{&env_names});
  env_var.append("value", CandidTypeVecText{&env_values});
  CandidTypeRecord settings;
  settings.append("controllers", CandidTypeVecPrincipal{&st.controllers});
  settings.append("freezing_threshold", CandidTypeNat{&st.freezing_threshold});
  settings.append_optional("compute_allocation",
                           CandidTypeNat{&st.compute_allocation});
  settings.append_optional("memory_allocation",
                           CandidTypeNat{&st.memory_allocation});
  settings.append_optional("reserved_cycles_limit",
                           CandidTypeNat{&st.reserved_cycles_limit});
  settings.append_optional(
      "minimum_incoming_canister_call_cycles",
      CandidTypeNat{&st.minimum_incoming_canister_call_cycles});
  settings.append_optional("log_visibility",
                           visibility_type(log_label, log_viewers));
  settings.append_optional("log_memory_limit",
                           CandidTypeNat{&st.log_memory_limit});
  settings.append_optional("snapshot_visibility",
                           visibility_type(snapshot_label, snapshot_viewers));
  settings.append_optional("status_visibility",
                           visibility_type(status_label, status_viewers));
  settings.append_optional("wasm_memory_limit",
                           CandidTypeNat{&st.wasm_memory_limit});
  settings.append_optional("wasm_memory_threshold",
                           CandidTypeNat{&st.wasm_memory_threshold});
  settings.append_optional("environment_variables",
                           CandidTypeVecRecord{&env_var});

  IC_MemoryMetrics &mm = status.memory_metrics;
  CandidTypeRecord memory_metrics;
  memory_metrics.append_optional("wasm_memory_size",
                                 CandidTypeNat{&mm.wasm_memory_size});
  memory_metrics.append_optional("stable_memory_size",
                                 CandidTypeNat{&mm.stable_memory_size});
  memory_metrics.append_optional("global_memory_size",
                                 CandidTypeNat{&mm.global_memory_size});
  memory_metrics.append_optional("wasm_binary_size",
                                 CandidTypeNat{&mm.wasm_binary_size});
  memory_metrics.append_optional("custom_sections_size",
                                 CandidTypeNat{&mm.custom_sections_size});
  memory_metrics.append_optional("canister_history_size",
                                 CandidTypeNat{&mm.canister_history_size});
  memory_metrics.append_optional("wasm_chunk_store_size",
                                 CandidTypeNat{&mm.wasm_chunk_store_size});
  memory_metrics.append_optional("snapshots_size",
                                 CandidTypeNat{&mm.snapshots_size});
  memory_metrics.append_optional("log_memory_store_size",
                                 CandidTypeNat{&mm.log_memory_store_size});

  IC_QueryStats &qs = status.query_stats;
  CandidTypeRecord query_stats;
  query_stats.append_optional("num_calls_total",
                              CandidTypeNat{&qs.num_calls_total});
  query_stats.append_optional("num_instructions_total",
                              CandidTypeNat{&qs.num_instructions_total});
  query_stats.append_optional("request_payload_bytes_total",
                              CandidTypeNat{&qs.request_payload_bytes_total});
  query_stats.append_optional("response_payload_bytes_total",
                              CandidTypeNat{&qs.response_payload_bytes_total});

  IC_Blob module_hash;
  bool has_module_hash{false};
  CandidTypeVecNat8 module_hash_vec{&module_hash};
  CandidTypeRecord result;
  result.append("status", status_variant);
  result.append("settings", settings);
  result.append("module_hash",
                CandidTypeOptVec{&module_hash_vec, &has_module_hash});
  result.append("memory_size", CandidTypeNat{&status.memory_size});
  result.append("cycles", CandidTypeNat{&status.cycles});
  result.append("idle_cycles_burned_per_day",
                CandidTypeNat{&status.idle_cycles_burned_per_day});
  result.append_optional("ready_for_migration",
                         CandidTypeBool{&status.ready_for_migration});
  result.append_optional("version", CandidTypeNat64{&status.version});
  result.append_optional("memory_metrics", memory_metrics);
  result.append_optional("reserved_cycles",
                         CandidTypeNat{&status.reserved_cycles});
  result.append_optional("query_stats", query_stats);
  r->decode(result);

  if (label == "stopping") status.status = IC_API::CanisterStatus::Stopping;
  else if (label == "stopped") status.status = IC_API::CanisterStatus::Stopped;
  if (has_module_hash) status.module_hash = module_hash;
  st.log_visibility = visibility_from(log_label, log_viewers);
  st.snapshot_visibility = visibility_from(snapshot_label, snapshot_viewers);
  st.status_visibility = visibility_from(status_label, status_viewers);
  for (size_t i = 0; i < env_names.size() && i < env_values.size(); ++i) {
    st.environment_variables.push_back({env_names[i], env_values[i]});
  }
  status.controllers = st.controllers;
  status.freezing_threshold = st.freezing_threshold;
  co_return status;
}

IC_Task<IC_CallResult<void>>
IC_Management::update_settings(CandidTypePrincipal canister_id,
                               IC_CanisterSettings settings) {
  // settings is not an opt here: it is sent also when no field is set
  CandidTypeRecord settings_record;
  append_settings(settings_record, settings);
  CandidTypeRecord arg;
  arg.append("canister_id", canister_id);
  arg.append("settings", settings_record);
  arg.append("sender_canister_version",
             CandidTypeOptNat64{ic0_canister_version()});
  IC_CallResult<IC_CallReply> r =
      co_await IC_Call::bounded_wait(management(), "update_settings")
          .with_arg(arg);
  if (!r) co_return r.error();
  co_return IC_CallResult<void>{};
}

IC_Task<IC_CallResult<void>>
IC_Management::start_canister(CandidTypePrincipal canister_id) {
  co_return co_await call_for_canister("start_canister", canister_id);
}

IC_Task<IC_CallResult<void>>
IC_Management::stop_canister(CandidTypePrincipal canister_id) {
  co_return co_await call_for_canister("stop_canister", canister_id);
}

IC_Task<IC_CallResult<void>>
IC_Management::delete_canister(CandidTypePrincipal canister_id) {
  co_return co_await call_for_canister("delete_canister", canister_id);
}

IC_Task<IC_CallResult<void>>
IC_Management::deposit_cycles(CandidTypePrincipal canister_id,
                              __uint128_t cycles) {
  CandidTypeRecord arg;
  arg.append("canister_id", canister_id);
  // The attached cycles are what is deposited, so the call must not time out
  IC_CallResult<IC_CallReply> r =
      co_await IC_Call::guaranteed_response(management(), "deposit_cycles")
          .with_arg(arg)
          .with_cycles(cycles);
  if (!r) co_return r.error();
  co_return IC_CallResult<void>{};
}

IC_Task<IC_CallResult<IC_PublicKey>> IC_Management::ecdsa_public_key(
    IC_EcdsaKeyId key, std::vector<IC_Blob> derivation_path,
    std::optional<CandidTypePrincipal> canister_id) {
  CandidTypeRecord arg;
  arg.append("canister_id",
             CandidTypeOptPrincipal{principal_text(canister_id)});
  arg.append("derivation_path", CandidTypeVecBlob{derivation_path});
  arg.append("key_id", ecdsa_key_id(key));
  co_return co_await call_for_public_key(
      IC_Call::bounded_wait(management(), "ecdsa_public_key").with_arg(arg));
}

IC_Task<IC_CallResult<IC_Blob>>
IC_Management::sign_with_ecdsa(IC_Blob message_hash,
                               std::vector<IC_Blob> derivation_path,
                               IC_EcdsaKeyId key) {
  IC_API::SignCost cost = IC_API::cost_sign_with_ecdsa(key.name, key.curve);
  if (cost.error != IC_API::SignCostError::None) {
    co_return sign_cost_error(cost.error);
  }
  CandidTypeRecord arg;
  arg.append("message_hash", CandidTypeVecNat8{message_hash});
  arg.append("derivation_path", CandidTypeVecBlob{derivation_path});
  arg.append("key_id", ecdsa_key_id(key));
  co_return co_await call_for_blob(
      IC_Call::guaranteed_response(management(), "sign_with_ecdsa")
          .with_arg(arg)
          .with_cycles(cost.cycles),
      "signature");
}

IC_Task<IC_CallResult<IC_PublicKey>> IC_Management::schnorr_public_key(
    IC_SchnorrKeyId key, std::vector<IC_Blob> derivation_path,
    std::optional<CandidTypePrincipal> canister_id) {
  CandidTypeRecord arg;
  arg.append("canister_id",
             CandidTypeOptPrincipal{principal_text(canister_id)});
  arg.append("derivation_path", CandidTypeVecBlob{derivation_path});
  arg.append("key_id", schnorr_key_id(key));
  co_return co_await call_for_public_key(
      IC_Call::bounded_wait(management(), "schnorr_public_key").with_arg(arg));
}

IC_Task<IC_CallResult<IC_Blob>> IC_Management::sign_with_schnorr(
    IC_Blob message, std::vector<IC_Blob> derivation_path, IC_SchnorrKeyId key,
    std::optional<IC_Blob> bip341_merkle_root_hash) {
  IC_API::SignCost cost =
      IC_API::cost_sign_with_schnorr(key.name, key.algorithm);
  if (cost.error != IC_API::SignCostError::None) {
    co_return sign_cost_error(cost.error);
  }
  CandidTypeRecord arg;
  arg.append("message", CandidTypeVecNat8{message});
  arg.append("derivation_path", CandidTypeVecBlob{derivation_path});
  arg.append("key_id", schnorr_key_id(key));
  if (bip341_merkle_root_hash.has_value()) {
    CandidTypeRecord bip341;
    bip341.append("merkle_root_hash",
                  CandidTypeVecNat8{*bip341_merkle_root_hash});
    arg.append("aux",
               CandidTypeOptVariant{CandidTypeVariant{"bip341", bip341}});
  }
  co_return co_await call_for_blob(
      IC_Call::guaranteed_response(management(), "sign_with_schnorr")
          .with_arg(arg)
          .with_cycles(cost.cycles),
      "signature");
}

IC_Task<IC_CallResult<IC_Blob>> IC_Management::vetkd_public_key(
    IC_VetKdKeyId key, IC_Blob context,
    std::optional<CandidTypePrincipal> canister_id) {
  CandidTypeRecord arg;
  arg.append("canister_id",
             CandidTypeOptPrincipal{principal_text(canister_id)});
  arg.append("context", CandidTypeVecNat8{context});
  arg.append("key_id", vetkd_key_id(key));
  co_return co_await call_for_blob(
      IC_Call::bounded_wait(management(), "vetkd_public_key").with_arg(arg),
      "public_key");
}

IC_Task<IC_CallResult<IC_Blob>>
IC_Management::vetkd_derive_key(IC_Blob input, IC_Blob context,
                                IC_Blob transport_public_key,
                                IC_VetKdKeyId key) {
  IC_API::SignCost cost = IC_API::cost_vetkd_derive_key(key.name, key.curve);
  if (cost.error != IC_API::SignCostError::None) {
    co_return sign_cost_error(cost.error);
  }
  CandidTypeRecord arg;
  arg.append("input", CandidTypeVecNat8{input});
  arg.append("context", CandidTypeVecNat8{context});
  arg.append("transport_public_key", CandidTypeVecNat8{transport_public_key});
  arg.append("key_id", vetkd_key_id(key));
  co_return co_await call_for_blob(
      IC_Call::guaranteed_response(management(), "vetkd_derive_key")
          .with_arg(arg)
          .with_cycles(cost.cycles),
      "encrypted_key");
}

IC_Task<IC_CallResult<IC_HttpOutcallResponse>>
IC_Management::http_request(IC_HttpOutcallRequest request) {
  std::vector<std::string> names, values;
  uint64_t request_size = request.url.size();
  for (const IC_HeaderField &h : request.headers) {
    names.push_back(h.name);
    values.push_back(h.value);
    request_size += h.name.size() + h.value.size();
  }
  CandidTypeRecord header;
  header.append("name", CandidTypeVecText{names});
  header.append("value", CandidTypeVecText{values});

  // The limit paid for is the limit sent, also when the IC's default applies.
  uint64_t max_response_bytes =
      request.max_response_bytes.value_or(HTTP_MAX_RESPONSE_BYTES);
  CandidTypeRecord arg;
  arg.append("url", CandidTypeText{request.url});
  arg.append("max_response_bytes", CandidTypeOptNat64{max_response_bytes});
  arg.append("method", CandidTypeVariant{http_method(request.method)});
  arg.append("headers", CandidTypeVecRecord{header});
  if (request.body.has_value()) {
    request_size += request.body->size();
    arg.append("body", CandidTypeOptVec{CandidTypeVecNat8{*request.body}});
  }
  if (request.transform_method.has_value()) {
    request_size +=
        request.transform_method->size() + request.transform_context.size();
    // func (record { response : http_request_result; context : blob })
    //   -> (http_request_result) query
    CandidTypeRecord transform_args;
    transform_args.append("response", http_result_type());
    transform_args.append("context", CandidTypeVecNat8{});
    CandidTypeFunc function{canister_self(), *request.transform_method};
    function.append_arg(transform_args);
    function.append_ret(http_result_type());
    function.set_query();
    CandidTypeRecord transform;
    transform.append("function", function);
    transform.append("context", CandidTypeVecNat8{request.transform_context});
    arg.append("transform", CandidTypeOptRecord{transform});
  }
  arg.append("is_replicated", CandidTypeOptBool{request.is_replicated});

  __uint128_t cycles =
      IC_API::cost_http_request(request_size, max_response_bytes);
  IC_CallResult<IC_CallReply> r =
      co_await IC_Call::guaranteed_response(management(), "http_request")
          .with_arg(arg)
          .with_cycles(cycles);
  if (!r) co_return r.error();

  IC_HttpOutcallResponse response;
  std::vector<std::string> r_names, r_values;
  CandidTypeRecord r_header;
  r_header.append("name", CandidTypeVecText{&r_names});
  r_header.append("value", CandidTypeVecText{&r_values});
  CandidTypeRecord result;
  result.append("status", CandidTypeNat{&response.status});
  result.append("headers", CandidTypeVecRecord{&r_header});
  result.append("body", CandidTypeVecNat8{&response.body});
  r->decode(result);
  for (size_t i = 0; i < r_names.size() && i < r_values.size(); ++i) {
    response.headers.push_back(IC_HeaderField{r_names[i], r_values[i]});
  }
  co_return response;
}

#endif // __cpp_impl_coroutine
