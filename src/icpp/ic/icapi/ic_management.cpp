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

// canister_install_mode
CandidTypeVariant install_mode_value(const IC_InstallMode &mode) {
  if (mode.kind == IC_InstallMode::Kind::Reinstall)
    return CandidTypeVariant{"reinstall"};
  if (mode.kind == IC_InstallMode::Kind::Upgrade) {
    // upgrade : opt record { skip_pre_upgrade : opt bool;
    //   wasm_memory_persistence : opt variant { keep; replace } }
    CandidTypeRecord options;
    if (mode.skip_pre_upgrade.has_value())
      options.append("skip_pre_upgrade",
                     CandidTypeOptBool{*mode.skip_pre_upgrade});
    if (mode.wasm_memory_persistence.has_value())
      options.append(
          "wasm_memory_persistence",
          CandidTypeOptVariant{CandidTypeVariant{
              *mode.wasm_memory_persistence == IC_WasmMemoryPersistence::Keep
                  ? "keep"
                  : "replace"}});
    return CandidTypeVariant{"upgrade", CandidTypeOptRecord{options}};
  }
  return CandidTypeVariant{"install"};
}

// The type of the change records of canister_info, decoded into change and
// the scratch labels; finish_change maps the labels
struct ChangeScratch {
  std::string origin_label;
  std::string details_label;
  std::string mode_label;
  std::string source_label;
  bool has_settings_controllers{false};
  std::vector<std::string> settings_controllers;
  bool has_creation_hash{false};
  IC_Blob creation_hash;
  bool has_settings_hash{false};
  IC_Blob settings_hash;
  // The vecs of the opt vec fields: a CandidTypeOptVec decodes through a
  // pointer to its vec, so they must live as long as the decode
  CandidTypeVecNat8 creation_hash_vec;
  CandidTypeVecPrincipal settings_controllers_vec;
  CandidTypeVecNat8 settings_hash_vec;
};

CandidType change_type(IC_Change &c, ChangeScratch &x) {
  IC_ChangeOrigin &o = c.origin;
  IC_ChangeDetails &d = c.details;

  CandidTypeRecord from_user;
  from_user.append("user_id", CandidTypePrincipal{&o.user_id});
  CandidTypeRecord from_canister;
  from_canister.append("canister_id", CandidTypePrincipal{&o.canister_id});
  from_canister.append("canister_version",
                       CandidTypeOptNat64{&o.canister_version});
  CandidTypeVariant origin{&x.origin_label};
  origin.append("from_user", from_user);
  origin.append("from_canister", from_canister);

  CandidTypeRecord creation;
  creation.append("controllers", CandidTypeVecPrincipal{&d.controllers});
  x.creation_hash_vec = CandidTypeVecNat8{&x.creation_hash};
  creation.append("environment_variables_hash",
                  CandidTypeOptVec{&x.creation_hash_vec, &x.has_creation_hash});

  CandidTypeVariant mode{&x.mode_label};
  mode.append("install", CandidTypeNull{});
  mode.append("reinstall", CandidTypeNull{});
  mode.append("upgrade", CandidTypeNull{});
  CandidTypeRecord code_deployment;
  code_deployment.append("mode", mode);
  code_deployment.append("module_hash", CandidTypeVecNat8{&d.module_hash});

  CandidTypeVariant source{&x.source_label};
  source.append("taken_from_canister", CandidTypeReserved{});
  source.append("metadata_upload", CandidTypeReserved{});
  CandidTypeRecord load_snapshot;
  load_snapshot.append("from_canister_id",
                       CandidTypeOptPrincipal{&d.from_canister_id});
  load_snapshot.append("snapshot_id", CandidTypeVecNat8{&d.snapshot_id});
  load_snapshot.append("canister_version",
                       CandidTypeNat64{&d.snapshot_canister_version});
  load_snapshot.append("taken_at_timestamp",
                       CandidTypeNat64{&d.taken_at_timestamp});
  load_snapshot.append_optional("source", source);

  CandidTypeRecord controllers_change;
  controllers_change.append("controllers",
                            CandidTypeVecPrincipal{&d.controllers});

  CandidTypeRecord settings_change;
  x.settings_controllers_vec = CandidTypeVecPrincipal{&x.settings_controllers};
  settings_change.append("controllers",
                         CandidTypeOptVec{&x.settings_controllers_vec,
                                          &x.has_settings_controllers});
  x.settings_hash_vec = CandidTypeVecNat8{&x.settings_hash};
  settings_change.append(
      "environment_variables_hash",
      CandidTypeOptVec{&x.settings_hash_vec, &x.has_settings_hash});

  CandidTypeRecord rename_to;
  rename_to.append("canister_id",
                   CandidTypePrincipal{&d.rename_to_canister_id});
  rename_to.append("version", CandidTypeNat64{&d.rename_to_version});
  rename_to.append("total_num_changes",
                   CandidTypeNat64{&d.rename_to_total_num_changes});
  CandidTypeRecord rename_canister;
  rename_canister.append("canister_id",
                         CandidTypePrincipal{&d.renamed_canister_id});
  rename_canister.append("total_num_changes",
                         CandidTypeNat64{&d.renamed_total_num_changes});
  rename_canister.append("rename_to", rename_to);
  rename_canister.append("requested_by", CandidTypePrincipal{&d.requested_by});

  // All the cases the replica sends, also settings_change, which is not (yet)
  // in the published spec: a case that is not declared traps
  CandidTypeVariant details{&x.details_label};
  details.append("creation", creation);
  details.append("code_uninstall", CandidTypeNull{});
  details.append("code_deployment", code_deployment);
  details.append("load_snapshot", load_snapshot);
  details.append("controllers_change", controllers_change);
  details.append("settings_change", settings_change);
  details.append("rename_canister", rename_canister);

  CandidTypeRecord change;
  change.append("timestamp_nanos", CandidTypeNat64{&c.timestamp_nanos});
  change.append("canister_version", CandidTypeNat64{&c.canister_version});
  change.append("origin", origin);
  change.append("details", details);
  return change;
}

void finish_change(IC_Change &c, const ChangeScratch &x) {
  c.origin.kind = x.origin_label == "from_canister"
                      ? IC_ChangeOrigin::Kind::FromCanister
                      : IC_ChangeOrigin::Kind::FromUser;
  using K = IC_ChangeDetails::Kind;
  IC_ChangeDetails &d = c.details;
  const std::string &l = x.details_label;
  d.kind = l == "code_uninstall"       ? K::CodeUninstall
           : l == "code_deployment"    ? K::CodeDeployment
           : l == "load_snapshot"      ? K::LoadSnapshot
           : l == "controllers_change" ? K::ControllersChange
           : l == "settings_change"    ? K::SettingsChange
           : l == "rename_canister"    ? K::RenameCanister
                                       : K::Creation;
  d.mode = x.mode_label == "reinstall" ? IC_InstallMode::Kind::Reinstall
           : x.mode_label == "upgrade" ? IC_InstallMode::Kind::Upgrade
                                       : IC_InstallMode::Kind::Install;
  d.snapshot_source = x.source_label == "metadata_upload"
                          ? IC_ChangeDetails::SnapshotSource::MetadataUpload
                          : IC_ChangeDetails::SnapshotSource::TakenFromCanister;
  if (x.has_settings_controllers)
    d.settings_controllers = x.settings_controllers;
  if (d.kind == K::Creation && x.has_creation_hash)
    d.environment_variables_hash = x.creation_hash;
  if (d.kind == K::SettingsChange && x.has_settings_hash)
    d.environment_variables_hash = x.settings_hash;
}

// An opt blob field: left out of the record when it has no value
void append_opt_blob(CandidTypeRecord &record, const char *name,
                     const std::optional<IC_Blob> &blob) {
  if (blob.has_value())
    record.append(name, CandidTypeOptVec{CandidTypeVecNat8{*blob}});
}

// record { id : blob; taken_at_timestamp : nat64; total_size : nat64 }
CandidTypeRecord snapshot_type(IC_Snapshot &snapshot) {
  CandidTypeRecord r;
  r.append("id", CandidTypeVecNat8{&snapshot.id});
  r.append("taken_at_timestamp", CandidTypeNat64{&snapshot.taken_at_timestamp});
  r.append("total_size", CandidTypeNat64{&snapshot.total_size});
  return r;
}

// variant { i32 : int32; i64 : int64; f32 : float32; f64 : float64; v128 : nat }
CandidTypeVariant global_value(const IC_SnapshotGlobal &g) {
  using K = IC_SnapshotGlobal::Kind;
  const char *label = g.kind == K::I64    ? "i64"
                      : g.kind == K::F32  ? "f32"
                      : g.kind == K::F64  ? "f64"
                      : g.kind == K::V128 ? "v128"
                                          : "i32";
  // Every case, so that all elements of the vec have the same type
  CandidTypeVariant v{std::string(label)};
  v.append("i32", CandidTypeInt32{g.i32});
  v.append("i64", CandidTypeInt64{g.i64});
  v.append("f32", CandidTypeFloat32{g.f32});
  v.append("f64", CandidTypeFloat64{g.f64});
  v.append("v128", CandidTypeNat{g.v128});
  return v;
}

const char *hook_status_label(IC_LowWasmMemoryHookStatus s) {
  switch (s) {
  case IC_LowWasmMemoryHookStatus::Ready:
    return "ready";
  case IC_LowWasmMemoryHookStatus::Executed:
    return "executed";
  case IC_LowWasmMemoryHookStatus::ConditionNotSatisfied:
    break;
  }
  return "condition_not_satisfied";
}

// read_canister_snapshot_data's kind (with_size) or
// upload_canister_snapshot_data's kind
CandidTypeVariant snapshot_data_kind(const IC_SnapshotData &kind,
                                     bool with_size) {
  using K = IC_SnapshotData::Kind;
  if (kind.kind == K::WasmChunk) {
    if (!with_size) return CandidTypeVariant{"wasm_chunk"};
    CandidTypeRecord chunk;
    chunk.append("hash", CandidTypeVecNat8{kind.hash});
    return CandidTypeVariant{"wasm_chunk", chunk};
  }
  CandidTypeRecord range;
  range.append("offset", CandidTypeNat64{kind.offset});
  if (with_size) range.append("size", CandidTypeNat64{kind.size});
  const char *label = kind.kind == K::WasmMemory     ? "wasm_memory"
                      : kind.kind == K::StableMemory ? "stable_memory"
                                                     : "wasm_module";
  return CandidTypeVariant{label, range};
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

IC_Task<IC_CallResult<void>>
IC_Management::install_code(CandidTypePrincipal canister_id,
                            IC_InstallMode mode, IC_Blob wasm_module,
                            IC_Blob arg) {
  CandidTypeRecord args;
  args.append("mode", install_mode_value(mode));
  args.append("canister_id", canister_id);
  args.append("wasm_module", CandidTypeVecNat8{wasm_module});
  args.append("arg", CandidTypeVecNat8{arg});
  args.append("sender_canister_version",
              CandidTypeOptNat64{ic0_canister_version()});
  IC_CallResult<IC_CallReply> r =
      co_await IC_Call::bounded_wait(management(), "install_code")
          .with_arg(args);
  if (!r) co_return r.error();
  co_return IC_CallResult<void>{};
}

IC_Task<IC_CallResult<IC_Blob>>
IC_Management::upload_chunk(CandidTypePrincipal canister_id, IC_Blob chunk) {
  CandidTypeRecord arg;
  arg.append("canister_id", canister_id);
  arg.append("chunk", CandidTypeVecNat8{chunk});
  co_return co_await call_for_blob(
      IC_Call::bounded_wait(management(), "upload_chunk").with_arg(arg),
      "hash");
}

IC_Task<IC_CallResult<void>>
IC_Management::clear_chunk_store(CandidTypePrincipal canister_id) {
  co_return co_await call_for_canister("clear_chunk_store", canister_id);
}

IC_Task<IC_CallResult<std::vector<IC_Blob>>>
IC_Management::stored_chunks(CandidTypePrincipal canister_id) {
  CandidTypeRecord arg;
  arg.append("canister_id", canister_id);
  IC_CallResult<IC_CallReply> r =
      co_await IC_Call::bounded_wait(management(), "stored_chunks")
          .with_arg(arg);
  if (!r) co_return r.error();
  // vec record { hash : blob }
  std::vector<IC_Blob> hashes;
  CandidTypeRecord chunk_hash;
  chunk_hash.append("hash", CandidTypeVecBlob{&hashes});
  r->decode(CandidTypeVecRecord{&chunk_hash});
  co_return hashes;
}

IC_Task<IC_CallResult<void>> IC_Management::install_chunked_code(
    CandidTypePrincipal target_canister, IC_InstallMode mode,
    std::vector<IC_Blob> chunk_hashes, IC_Blob wasm_module_hash, IC_Blob arg,
    std::optional<CandidTypePrincipal> store_canister) {
  CandidTypeRecord chunk_hash;
  chunk_hash.append("hash", CandidTypeVecBlob{chunk_hashes});
  CandidTypeRecord args;
  args.append("mode", install_mode_value(mode));
  args.append("target_canister", target_canister);
  args.append("store_canister",
              CandidTypeOptPrincipal{principal_text(store_canister)});
  args.append("chunk_hashes_list", CandidTypeVecRecord{chunk_hash});
  args.append("wasm_module_hash", CandidTypeVecNat8{wasm_module_hash});
  args.append("arg", CandidTypeVecNat8{arg});
  args.append("sender_canister_version",
              CandidTypeOptNat64{ic0_canister_version()});
  IC_CallResult<IC_CallReply> r =
      co_await IC_Call::bounded_wait(management(), "install_chunked_code")
          .with_arg(args);
  if (!r) co_return r.error();
  co_return IC_CallResult<void>{};
}

IC_Task<IC_CallResult<void>>
IC_Management::uninstall_code(CandidTypePrincipal canister_id) {
  CandidTypeRecord arg;
  arg.append("canister_id", canister_id);
  arg.append("sender_canister_version",
             CandidTypeOptNat64{ic0_canister_version()});
  IC_CallResult<IC_CallReply> r =
      co_await IC_Call::bounded_wait(management(), "uninstall_code")
          .with_arg(arg);
  if (!r) co_return r.error();
  co_return IC_CallResult<void>{};
}

IC_Task<IC_CallResult<IC_Blob>>
IC_Management::canister_metadata(CandidTypePrincipal canister_id,
                                 std::string name) {
  CandidTypeRecord arg;
  arg.append("canister_id", canister_id);
  arg.append("name", CandidTypeText{name});
  co_return co_await call_for_blob(
      IC_Call::bounded_wait(management(), "canister_metadata").with_arg(arg),
      "value");
}

IC_Task<IC_CallResult<IC_CanisterInfo>>
IC_Management::canister_info(CandidTypePrincipal canister_id,
                             std::optional<uint64_t> num_requested_changes) {
  CandidTypeRecord arg;
  arg.append("canister_id", canister_id);
  arg.append("num_requested_changes",
             CandidTypeOptNat64{num_requested_changes});
  IC_CallResult<IC_CallReply> r =
      co_await IC_Call::bounded_wait(management(), "canister_info")
          .with_arg(arg);
  if (!r) co_return r.error();

  IC_CanisterInfo info;
  IC_Change change;
  ChangeScratch scratch;
  IC_Blob module_hash;
  bool has_module_hash{false};
  CandidTypeVecNat8 module_hash_vec{&module_hash};
  CandidTypeRecord result;
  result.append("total_num_changes", CandidTypeNat64{&info.total_num_changes});
  result.append("recent_changes", CandidTypeVecOf(
                                      [&]() -> CandidType {
                                        change = {};
                                        scratch = {};
                                        return change_type(change, scratch);
                                      },
                                      [&]() {
                                        finish_change(change, scratch);
                                        info.recent_changes.push_back(change);
                                      }));
  result.append("module_hash",
                CandidTypeOptVec{&module_hash_vec, &has_module_hash});
  result.append("controllers", CandidTypeVecPrincipal{&info.controllers});
  r->decode(result);
  if (has_module_hash) info.module_hash = module_hash;
  co_return info;
}

IC_Task<IC_CallResult<IC_Snapshot>>
IC_Management::take_canister_snapshot(CandidTypePrincipal canister_id,
                                      std::optional<IC_Blob> replace_snapshot,
                                      std::optional<bool> uninstall_code) {
  CandidTypeRecord arg;
  arg.append("canister_id", canister_id);
  append_opt_blob(arg, "replace_snapshot", replace_snapshot);
  if (uninstall_code.has_value())
    arg.append("uninstall_code", CandidTypeOptBool{*uninstall_code});
  arg.append("sender_canister_version",
             CandidTypeOptNat64{ic0_canister_version()});
  IC_CallResult<IC_CallReply> r =
      co_await IC_Call::bounded_wait(management(), "take_canister_snapshot")
          .with_arg(arg);
  if (!r) co_return r.error();
  IC_Snapshot snapshot;
  r->decode(snapshot_type(snapshot));
  co_return snapshot;
}

IC_Task<IC_CallResult<void>>
IC_Management::load_canister_snapshot(CandidTypePrincipal canister_id,
                                      IC_Blob snapshot_id) {
  CandidTypeRecord arg;
  arg.append("canister_id", canister_id);
  arg.append("snapshot_id", CandidTypeVecNat8{snapshot_id});
  arg.append("sender_canister_version",
             CandidTypeOptNat64{ic0_canister_version()});
  IC_CallResult<IC_CallReply> r =
      co_await IC_Call::bounded_wait(management(), "load_canister_snapshot")
          .with_arg(arg);
  if (!r) co_return r.error();
  co_return IC_CallResult<void>{};
}

IC_Task<IC_CallResult<std::vector<IC_Snapshot>>>
IC_Management::list_canister_snapshots(CandidTypePrincipal canister_id) {
  CandidTypeRecord arg;
  arg.append("canister_id", canister_id);
  IC_CallResult<IC_CallReply> r =
      co_await IC_Call::bounded_wait(management(), "list_canister_snapshots")
          .with_arg(arg);
  if (!r) co_return r.error();
  std::vector<IC_Snapshot> snapshots;
  IC_Snapshot snapshot;
  r->decode(CandidTypeVecOf(
      [&]() -> CandidType {
        snapshot = {};
        return snapshot_type(snapshot);
      },
      [&]() { snapshots.push_back(snapshot); }));
  co_return snapshots;
}

IC_Task<IC_CallResult<void>>
IC_Management::delete_canister_snapshot(CandidTypePrincipal canister_id,
                                        IC_Blob snapshot_id) {
  CandidTypeRecord arg;
  arg.append("canister_id", canister_id);
  arg.append("snapshot_id", CandidTypeVecNat8{snapshot_id});
  IC_CallResult<IC_CallReply> r =
      co_await IC_Call::bounded_wait(management(), "delete_canister_snapshot")
          .with_arg(arg);
  if (!r) co_return r.error();
  co_return IC_CallResult<void>{};
}

IC_Task<IC_CallResult<IC_SnapshotMetadata>>
IC_Management::read_canister_snapshot_metadata(CandidTypePrincipal canister_id,
                                               IC_Blob snapshot_id) {
  CandidTypeRecord arg;
  arg.append("canister_id", canister_id);
  arg.append("snapshot_id", CandidTypeVecNat8{snapshot_id});
  IC_CallResult<IC_CallReply> r =
      co_await IC_Call::bounded_wait(management(),
                                     "read_canister_snapshot_metadata")
          .with_arg(arg);
  if (!r) co_return r.error();

  IC_SnapshotMetadata md;
  std::string source_label;
  CandidTypeVariant source{&source_label};
  source.append("taken_from_canister", CandidTypeReserved{});
  source.append("metadata_upload", CandidTypeReserved{});

  IC_SnapshotGlobal global;
  std::string global_label;
  CandidTypeVecOf globals(
      [&]() -> CandidType {
        global = {};
        global_label.clear();
        CandidTypeVariant v{&global_label};
        v.append("i32", CandidTypeInt32{&global.i32});
        v.append("i64", CandidTypeInt64{&global.i64});
        v.append("f32", CandidTypeFloat32{&global.f32});
        v.append("f64", CandidTypeFloat64{&global.f64});
        v.append("v128", CandidTypeNat{&global.v128});
        return v;
      },
      [&]() {
        using K = IC_SnapshotGlobal::Kind;
        global.kind = global_label == "i64"    ? K::I64
                      : global_label == "f32"  ? K::F32
                      : global_label == "f64"  ? K::F64
                      : global_label == "v128" ? K::V128
                                               : K::I32;
        md.globals.push_back(global);
      });

  CandidTypeRecord chunk_hash;
  chunk_hash.append("hash", CandidTypeVecBlob{&md.wasm_chunk_store});

  std::string timer_label;
  uint64_t timer_timestamp{0};
  bool has_timer{false};
  CandidTypeVariant timer{&timer_label};
  timer.append("inactive", CandidTypeNull{});
  timer.append("active", CandidTypeNat64{&timer_timestamp});

  std::string hook_label;
  bool has_hook{false};
  CandidTypeVariant hook{&hook_label};
  hook.append("condition_not_satisfied", CandidTypeNull{});
  hook.append("ready", CandidTypeNull{});
  hook.append("executed", CandidTypeNull{});

  CandidTypeRecord result;
  result.append_optional("source", source);
  result.append("taken_at_timestamp", CandidTypeNat64{&md.taken_at_timestamp});
  result.append("wasm_module_size", CandidTypeNat64{&md.wasm_module_size});
  result.append("globals", globals);
  result.append("wasm_memory_size", CandidTypeNat64{&md.wasm_memory_size});
  result.append("stable_memory_size", CandidTypeNat64{&md.stable_memory_size});
  result.append("wasm_chunk_store", CandidTypeVecRecord{&chunk_hash});
  result.append("canister_version", CandidTypeNat64{&md.canister_version});
  result.append("certified_data", CandidTypeVecNat8{&md.certified_data});
  result.append("global_timer", CandidTypeOptVariant{&timer, &has_timer});
  result.append("on_low_wasm_memory_hook_status",
                CandidTypeOptVariant{&hook, &has_hook});
  r->decode(result);

  if (source_label == "metadata_upload")
    md.source = IC_ChangeDetails::SnapshotSource::MetadataUpload;
  if (has_timer)
    md.global_timer = IC_GlobalTimer{timer_label == "active", timer_timestamp};
  if (has_hook) {
    md.on_low_wasm_memory_hook_status =
        hook_label == "ready" ? IC_LowWasmMemoryHookStatus::Ready
        : hook_label == "executed"
            ? IC_LowWasmMemoryHookStatus::Executed
            : IC_LowWasmMemoryHookStatus::ConditionNotSatisfied;
  }
  co_return md;
}

IC_Task<IC_CallResult<IC_Blob>>
IC_Management::read_canister_snapshot_data(CandidTypePrincipal canister_id,
                                           IC_Blob snapshot_id,
                                           IC_SnapshotData kind) {
  CandidTypeRecord arg;
  arg.append("canister_id", canister_id);
  arg.append("snapshot_id", CandidTypeVecNat8{snapshot_id});
  arg.append("kind", snapshot_data_kind(kind, true));
  co_return co_await call_for_blob(
      IC_Call::bounded_wait(management(), "read_canister_snapshot_data")
          .with_arg(arg),
      "chunk");
}

IC_Task<IC_CallResult<IC_Blob>>
IC_Management::upload_canister_snapshot_metadata(
    CandidTypePrincipal canister_id, IC_SnapshotMetadata metadata,
    std::optional<IC_Blob> replace_snapshot) {
  std::vector<CandidType> globals;
  for (const auto &g : metadata.globals)
    globals.push_back(global_value(g));
  CandidTypeRecord arg;
  arg.append("canister_id", canister_id);
  append_opt_blob(arg, "replace_snapshot", replace_snapshot);
  arg.append("wasm_module_size", CandidTypeNat64{metadata.wasm_module_size});
  arg.append("globals",
             CandidTypeVecOf(global_value(IC_SnapshotGlobal{}), globals));
  arg.append("wasm_memory_size", CandidTypeNat64{metadata.wasm_memory_size});
  arg.append("stable_memory_size",
             CandidTypeNat64{metadata.stable_memory_size});
  arg.append("certified_data", CandidTypeVecNat8{metadata.certified_data});
  if (metadata.global_timer.has_value()) {
    arg.append(
        "global_timer",
        CandidTypeOptVariant{
            metadata.global_timer->active
                ? CandidTypeVariant{"active",
                                    CandidTypeNat64{
                                        metadata.global_timer->timestamp}}
                : CandidTypeVariant{"inactive"}});
  }
  if (metadata.on_low_wasm_memory_hook_status.has_value()) {
    arg.append("on_low_wasm_memory_hook_status",
               CandidTypeOptVariant{CandidTypeVariant{hook_status_label(
                   *metadata.on_low_wasm_memory_hook_status)}});
  }
  co_return co_await call_for_blob(
      IC_Call::bounded_wait(management(), "upload_canister_snapshot_metadata")
          .with_arg(arg),
      "snapshot_id");
}

IC_Task<IC_CallResult<void>> IC_Management::upload_canister_snapshot_data(
    CandidTypePrincipal canister_id, IC_Blob snapshot_id, IC_SnapshotData kind,
    IC_Blob chunk) {
  CandidTypeRecord arg;
  arg.append("canister_id", canister_id);
  arg.append("snapshot_id", CandidTypeVecNat8{snapshot_id});
  arg.append("kind", snapshot_data_kind(kind, false));
  arg.append("chunk", CandidTypeVecNat8{chunk});
  IC_CallResult<IC_CallReply> r =
      co_await IC_Call::bounded_wait(management(),
                                     "upload_canister_snapshot_data")
          .with_arg(arg);
  if (!r) co_return r.error();
  co_return IC_CallResult<void>{};
}

IC_Task<IC_CallResult<std::vector<IC_CanisterLogRecord>>>
IC_Management::fetch_canister_logs(CandidTypePrincipal canister_id,
                                   std::optional<IC_LogFilter> filter) {
  CandidTypeRecord arg;
  arg.append("canister_id", canister_id);
  if (filter.has_value()) {
    CandidTypeRecord range;
    range.append("start", CandidTypeNat64{filter->start});
    range.append("end", CandidTypeNat64{filter->end});
    arg.append("filter",
               CandidTypeOptVariant{CandidTypeVariant{
                   filter->kind == IC_LogFilter::Kind::ByTimestampNanos
                       ? "by_timestamp_nanos"
                       : "by_idx",
                   range}});
  }
  IC_CallResult<IC_CallReply> r =
      co_await IC_Call::bounded_wait(management(), "fetch_canister_logs")
          .with_arg(arg);
  if (!r) co_return r.error();
  std::vector<uint64_t> idx, timestamp_nanos;
  std::vector<IC_Blob> content;
  CandidTypeRecord record;
  record.append("idx", CandidTypeVecNat64{&idx});
  record.append("timestamp_nanos", CandidTypeVecNat64{&timestamp_nanos});
  record.append("content", CandidTypeVecBlob{&content});
  CandidTypeRecord result;
  result.append("canister_log_records", CandidTypeVecRecord{&record});
  r->decode(result);
  std::vector<IC_CanisterLogRecord> records;
  for (size_t i = 0; i < idx.size(); ++i) {
    records.push_back({idx[i], timestamp_nanos[i], content[i]});
  }
  co_return records;
}

IC_Task<IC_CallResult<IC_CanisterMetrics>>
IC_Management::canister_metrics(CandidTypePrincipal canister_id) {
  CandidTypeRecord arg;
  arg.append("canister_id", canister_id);
  IC_CallResult<IC_CallReply> r =
      co_await IC_Call::bounded_wait(management(), "canister_metrics")
          .with_arg(arg);
  if (!r) co_return r.error();
  IC_CanisterMetrics metrics;
  IC_CyclesConsumed &c = metrics.cycles_consumed;
  CandidTypeRecord consumed;
  consumed.append_optional("memory", CandidTypeNat{&c.memory});
  consumed.append_optional("compute_allocation",
                           CandidTypeNat{&c.compute_allocation});
  consumed.append_optional("ingress_induction",
                           CandidTypeNat{&c.ingress_induction});
  consumed.append_optional("instructions", CandidTypeNat{&c.instructions});
  consumed.append_optional("request_and_response_transmission",
                           CandidTypeNat{&c.request_and_response_transmission});
  consumed.append_optional("uninstall", CandidTypeNat{&c.uninstall});
  consumed.append_optional("canister_creation",
                           CandidTypeNat{&c.canister_creation});
  consumed.append_optional("http_outcalls", CandidTypeNat{&c.http_outcalls});
  consumed.append_optional("burned_cycles", CandidTypeNat{&c.burned_cycles});
  CandidTypeRecord result;
  result.append("cycles_consumed", consumed);
  r->decode(result);
  co_return metrics;
}

IC_Task<IC_CallResult<IC_SubnetInfo>>
IC_Management::subnet_info(CandidTypePrincipal subnet_id) {
  CandidTypeRecord arg;
  arg.append("subnet_id", subnet_id);
  IC_CallResult<IC_CallReply> r =
      co_await IC_Call::bounded_wait(management(), "subnet_info").with_arg(arg);
  if (!r) co_return r.error();
  IC_SubnetInfo info;
  CandidTypeRecord result;
  result.append("replica_version", CandidTypeText{&info.replica_version});
  result.append_optional("registry_version",
                         CandidTypeNat64{&info.registry_version});
  r->decode(result);
  co_return info;
}

IC_Task<IC_CallResult<IC_SubnetMetrics>>
IC_Management::subnet_metrics(CandidTypePrincipal subnet_id) {
  CandidTypeRecord arg;
  arg.append("subnet_id", subnet_id);
  IC_CallResult<IC_CallReply> r =
      co_await IC_Call::bounded_wait(management(), "subnet_metrics")
          .with_arg(arg);
  if (!r) co_return r.error();
  // Experimental in the IC: every field may be absent
  IC_SubnetMetrics m;
  CandidTypeRecord result;
  result.append_optional("block_height", CandidTypeNat{&m.block_height});
  result.append_optional("num_canisters", CandidTypeNat{&m.num_canisters});
  result.append_optional("canister_state_bytes",
                         CandidTypeNat{&m.canister_state_bytes});
  result.append_optional("consumed_cycles_total",
                         CandidTypeNat{&m.consumed_cycles_total});
  result.append_optional("update_transactions_total",
                         CandidTypeNat{&m.update_transactions_total});
  result.append_optional("million_round_instructions_total",
                         CandidTypeNat{&m.million_round_instructions_total});
  r->decode(result);
  co_return m;
}

IC_Task<IC_CallResult<std::vector<IC_NodeMetricsHistoryEntry>>>
IC_Management::node_metrics_history(CandidTypePrincipal subnet_id,
                                    uint64_t start_at_timestamp_nanos) {
  CandidTypeRecord arg;
  arg.append("subnet_id", subnet_id);
  arg.append("start_at_timestamp_nanos",
             CandidTypeNat64{start_at_timestamp_nanos});
  IC_CallResult<IC_CallReply> r =
      co_await IC_Call::bounded_wait(management(), "node_metrics_history")
          .with_arg(arg);
  if (!r) co_return r.error();
  // vec record { timestamp_nanos : nat64; node_metrics : vec node_metrics }
  std::vector<IC_NodeMetricsHistoryEntry> history;
  uint64_t timestamp_nanos{0};
  std::vector<std::string> node_ids;
  std::vector<uint64_t> proposed, failures;
  // A CandidTypeVecRecord decodes through a pointer to its record, so the
  // record must live as long as the decode
  CandidTypeRecord node_metrics;
  node_metrics.append("node_id", CandidTypeVecPrincipal{&node_ids});
  node_metrics.append("num_blocks_proposed_total",
                      CandidTypeVecNat64{&proposed});
  node_metrics.append("num_block_failures_total",
                      CandidTypeVecNat64{&failures});
  r->decode(CandidTypeVecOf(
      [&]() -> CandidType {
        timestamp_nanos = 0;
        node_ids.clear();
        proposed.clear();
        failures.clear();
        CandidTypeRecord entry;
        entry.append("timestamp_nanos", CandidTypeNat64{&timestamp_nanos});
        entry.append("node_metrics", CandidTypeVecRecord{&node_metrics});
        return entry;
      },
      [&]() {
        IC_NodeMetricsHistoryEntry entry{timestamp_nanos, {}};
        for (size_t i = 0; i < node_ids.size(); ++i) {
          entry.node_metrics.push_back({node_ids[i], proposed[i], failures[i]});
        }
        history.push_back(entry);
      }));
  co_return history;
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
