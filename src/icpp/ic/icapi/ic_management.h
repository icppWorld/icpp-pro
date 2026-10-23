// Typed helpers for the management canister (aaaaa-aa), as coroutines:
//
//   IC_Async my_method_async() {
//     IC_API ic_api(CanisterUpdate{"my_method"}, false);
//     IC_CallResult<std::vector<uint8_t>> r = co_await IC_Management::raw_rand();
//     ...
//   }
//
// Each helper attaches the cycles its call costs (cost_create_canister,
// cost_sign_with_ecdsa, ...). A call that attaches cycles is a
// guaranteed-response call, like in the Rust CDK: the management canister is
// trusted, and a bounded-wait call could lose the cycles on a timeout. The
// others are bounded-wait calls.
//
// Not available in C++17 builds, which have no coroutines.

#pragma once

#if defined(__cpp_impl_coroutine)

#include <optional>
#include <string>
#include <vector>

// docs start: ic_management
// Who may read a canister's logs, snapshots or status
struct IC_Visibility {
  enum class Kind { Controllers, Public, AllowedViewers };
  Kind kind{Kind::Controllers};
  std::vector<std::string> allowed_viewers; // principal texts, AllowedViewers
};

struct IC_EnvironmentVariable {
  std::string name;
  std::string value;
};

// The settings of a canister, for create_canister and update_settings; a
// field left empty is not changed (create_canister: gets the IC's default)
struct IC_CanisterSettings {
  std::optional<std::vector<std::string>> controllers; // principal texts
  std::optional<__uint128_t> compute_allocation;
  std::optional<__uint128_t> memory_allocation;
  std::optional<__uint128_t> freezing_threshold;
  std::optional<__uint128_t> reserved_cycles_limit;
  std::optional<__uint128_t> wasm_memory_limit;
  std::optional<__uint128_t> minimum_incoming_canister_call_cycles;
  std::optional<IC_Visibility> log_visibility;
  std::optional<__uint128_t> log_memory_limit;
  std::optional<IC_Visibility> snapshot_visibility;
  std::optional<IC_Visibility> status_visibility;
  std::optional<__uint128_t> wasm_memory_threshold;
  std::optional<std::vector<IC_EnvironmentVariable>> environment_variables;
};

// The settings of a canister, as canister_status reports them
struct IC_DefiniteCanisterSettings {
  std::vector<std::string> controllers; // principal texts
  __uint128_t compute_allocation{0};
  __uint128_t memory_allocation{0};
  __uint128_t freezing_threshold{0};
  __uint128_t reserved_cycles_limit{0};
  __uint128_t minimum_incoming_canister_call_cycles{0};
  IC_Visibility log_visibility;
  __uint128_t log_memory_limit{0};
  IC_Visibility snapshot_visibility;
  IC_Visibility status_visibility;
  __uint128_t wasm_memory_limit{0};
  __uint128_t wasm_memory_threshold{0};
  std::vector<IC_EnvironmentVariable> environment_variables;
};

// The memory of a canister, in bytes, as canister_status reports it
struct IC_MemoryMetrics {
  __uint128_t wasm_memory_size{0};
  __uint128_t stable_memory_size{0};
  __uint128_t global_memory_size{0};
  __uint128_t wasm_binary_size{0};
  __uint128_t custom_sections_size{0};
  __uint128_t canister_history_size{0};
  __uint128_t wasm_chunk_store_size{0};
  __uint128_t snapshots_size{0};
  __uint128_t log_memory_store_size{0};
};

// The query calls a canister answered, as canister_status reports them
struct IC_QueryStats {
  __uint128_t num_calls_total{0};
  __uint128_t num_instructions_total{0};
  __uint128_t request_payload_bytes_total{0};
  __uint128_t response_payload_bytes_total{0};
};

// The threshold keys, e.g. "key_1" on mainnet, "test_key_1" (mainnet and a
// local network) and "dfx_test_key" (a local network)
struct IC_EcdsaKeyId {
  IC_API::EcdsaCurve curve{IC_API::EcdsaCurve::Secp256k1};
  std::string name;
};
struct IC_SchnorrKeyId {
  IC_API::SchnorrAlgorithm algorithm{IC_API::SchnorrAlgorithm::Ed25519};
  std::string name;
};
struct IC_VetKdKeyId {
  IC_API::VetKdCurve curve{IC_API::VetKdCurve::Bls12_381_G2};
  std::string name;
};

struct IC_PublicKey {
  std::vector<uint8_t> public_key;
  std::vector<uint8_t> chain_code;
};

using IC_Blob = std::vector<uint8_t>;

// The status of a canister, from IC_Management::canister_status. A field the
// IC added recently stays at its default when an older replica leaves it out.
struct IC_CanisterStatus {
  IC_API::CanisterStatus status{IC_API::CanisterStatus::Running};
  __uint128_t cycles{0};
  __uint128_t idle_cycles_burned_per_day{0};
  __uint128_t memory_size{0};
  std::optional<IC_Blob> module_hash; // empty: no code installed
  std::vector<std::string>
      controllers;                   // principal texts, = settings.controllers
  __uint128_t freezing_threshold{0}; // = settings.freezing_threshold
  bool ready_for_migration{false};
  uint64_t version{0}; // the canister version
  IC_DefiniteCanisterSettings settings;
  IC_MemoryMetrics memory_metrics;
  __uint128_t reserved_cycles{0};
  IC_QueryStats query_stats;
};

// How install_code and install_chunked_code install a module: Install into an
// empty canister, Reinstall (the state is cleared), or Upgrade (the state is
// kept), with the upgrade's options
enum class IC_WasmMemoryPersistence { Keep, Replace };
struct IC_InstallMode {
  enum class Kind { Install, Reinstall, Upgrade };
  Kind kind{Kind::Install};
  std::optional<bool> skip_pre_upgrade;                            // Upgrade
  std::optional<IC_WasmMemoryPersistence> wasm_memory_persistence; // Upgrade
};

// Where a change of a canister came from, in its history (canister_info)
struct IC_ChangeOrigin {
  enum class Kind { FromUser, FromCanister };
  Kind kind{Kind::FromUser};
  std::string user_id;                      // FromUser
  std::string canister_id;                  // FromCanister
  std::optional<uint64_t> canister_version; // FromCanister
};

// What a change of a canister was, in its history (canister_info). A new kind
// that the IC adds traps until icpp-pro knows it.
struct IC_ChangeDetails {
  enum class Kind {
    Creation,
    CodeUninstall,
    CodeDeployment,
    LoadSnapshot,
    ControllersChange,
    SettingsChange,
    RenameCanister
  };
  Kind kind{Kind::Creation};
  // Creation, ControllersChange
  std::vector<std::string> controllers;
  // SettingsChange: the controllers, when they changed
  std::optional<std::vector<std::string>> settings_controllers;
  // Creation, SettingsChange
  std::optional<IC_Blob> environment_variables_hash;
  // CodeDeployment
  IC_InstallMode::Kind mode{IC_InstallMode::Kind::Install};
  IC_Blob module_hash;
  // LoadSnapshot
  enum class SnapshotSource { TakenFromCanister, MetadataUpload };
  std::optional<std::string> from_canister_id;
  IC_Blob snapshot_id;
  uint64_t snapshot_canister_version{0};
  uint64_t taken_at_timestamp{0};
  SnapshotSource snapshot_source{SnapshotSource::TakenFromCanister};
  // RenameCanister
  std::string renamed_canister_id;
  uint64_t renamed_total_num_changes{0};
  std::string rename_to_canister_id;
  uint64_t rename_to_version{0};
  uint64_t rename_to_total_num_changes{0};
  std::string requested_by;
};

struct IC_Change {
  uint64_t timestamp_nanos{0};
  uint64_t canister_version{0};
  IC_ChangeOrigin origin;
  IC_ChangeDetails details;
};

// The public information of any canister, from canister_info
struct IC_CanisterInfo {
  uint64_t total_num_changes{0};
  std::vector<IC_Change> recent_changes; // oldest first
  std::optional<IC_Blob> module_hash;    // empty: no code installed
  std::vector<std::string> controllers;  // principal texts
};

// A snapshot of a canister: its wasm module, memories, chunk store and some
// of its state, taken with take_canister_snapshot
struct IC_Snapshot {
  IC_Blob id;
  uint64_t taken_at_timestamp{0}; // nanoseconds since 1970
  uint64_t total_size{0};         // bytes
};

// A wasm global of a snapshot
struct IC_SnapshotGlobal {
  enum class Kind { I32, I64, F32, F64, V128 };
  Kind kind{Kind::I32};
  int32_t i32{0};
  int64_t i64{0};
  float f32{0};
  double f64{0};
  __uint128_t v128{0};
};

struct IC_GlobalTimer {
  bool active{false};
  uint64_t timestamp{0}; // nanoseconds since 1970, when active
};

enum class IC_LowWasmMemoryHookStatus {
  ConditionNotSatisfied,
  Ready,
  Executed
};

// The metadata of a snapshot, from read_canister_snapshot_metadata; also the
// argument of upload_canister_snapshot_metadata, which uses its sizes,
// globals, certified data, global timer and hook status
struct IC_SnapshotMetadata {
  IC_ChangeDetails::SnapshotSource source{
      IC_ChangeDetails::SnapshotSource::TakenFromCanister};
  uint64_t taken_at_timestamp{0};
  uint64_t wasm_module_size{0};
  std::vector<IC_SnapshotGlobal> globals;
  uint64_t wasm_memory_size{0};
  uint64_t stable_memory_size{0};
  std::vector<IC_Blob> wasm_chunk_store; // the hashes of its chunks
  uint64_t canister_version{0};
  IC_Blob certified_data;
  std::optional<IC_GlobalTimer> global_timer;
  std::optional<IC_LowWasmMemoryHookStatus> on_low_wasm_memory_hook_status;
};

// Which part of a snapshot to read or upload: a range of the wasm module, the
// wasm memory or the stable memory (read: offset and size; upload: offset),
// or a chunk of the chunk store (read: hash)
struct IC_SnapshotData {
  enum class Kind { WasmModule, WasmMemory, StableMemory, WasmChunk };
  Kind kind{Kind::WasmModule};
  uint64_t offset{0};
  uint64_t size{0};
  IC_Blob hash;
};

// Calls to the management canister (aaaaa-aa), the IC's system API. Each
// helper encodes the Candid argument, attaches the cycles the call costs, and
// decodes the reply.
class IC_Management {
public:
  // 32 random bytes from the IC's random beacon, which no node can predict.
  // Execution is deterministic, so this is a canister's only source of secure
  // randomness.
  static IC_Task<IC_CallResult<IC_Blob>> raw_rand();

  // Creates an empty canister, e.g. one per user, controlled by this
  // canister unless settings say otherwise. It starts with extra_cycles, attached on top of the cost of
  // the creation: a canister needs cycles to allocate memory, so give it some
  // (the IC rejects the creation with an "insufficient cycles" error
  // otherwise).
  static IC_Task<IC_CallResult<CandidTypePrincipal>>
  create_canister(IC_CanisterSettings settings = {},
                  __uint128_t extra_cycles = 0);

  // The status of a canister: running or stopped, its cycles balance and
  // burn rate, its memory, its module hash and its controllers. A canister
  // may read its own status and that of a canister it controls; other
  // canisters only when the target's status_visibility allows it. The IC
  // rejects the call otherwise.
  static IC_Task<IC_CallResult<IC_CanisterStatus>>
  canister_status(CandidTypePrincipal canister_id);

  // Changes the settings of a canister this canister controls. Only the
  // fields that are set change.
  static IC_Task<IC_CallResult<void>>
  update_settings(CandidTypePrincipal canister_id,
                  IC_CanisterSettings settings);

  // Starts, stops and deletes a canister this canister controls. A stop waits
  // for the canister's calls in flight; a canister must be stopped before it
  // is deleted, and its remaining cycles are then lost.
  static IC_Task<IC_CallResult<void>>
  start_canister(CandidTypePrincipal canister_id);
  static IC_Task<IC_CallResult<void>>
  stop_canister(CandidTypePrincipal canister_id);
  static IC_Task<IC_CallResult<void>>
  delete_canister(CandidTypePrincipal canister_id);

  // Moves cycles from this canister to any canister, e.g. a top-up
  static IC_Task<IC_CallResult<void>>
  deposit_cycles(CandidTypePrincipal canister_id, __uint128_t cycles);

  // Installs a wasm module (raw, or gzip-compressed) into a canister this
  // canister controls, with arg as the init or post_upgrade argument (Candid
  // bytes). A module larger than about 2 MiB goes through the chunk store:
  // upload_chunk, then install_chunked_code.
  static IC_Task<IC_CallResult<void>>
  install_code(CandidTypePrincipal canister_id, IC_InstallMode mode,
               IC_Blob wasm_module, IC_Blob arg = {});

  // The chunk store of a canister (at most 1 MiB per chunk): upload_chunk
  // returns the chunk's sha256 hash, stored_chunks lists the hashes.
  static IC_Task<IC_CallResult<IC_Blob>>
  upload_chunk(CandidTypePrincipal canister_id, IC_Blob chunk);
  static IC_Task<IC_CallResult<void>>
  clear_chunk_store(CandidTypePrincipal canister_id);
  static IC_Task<IC_CallResult<std::vector<IC_Blob>>>
  stored_chunks(CandidTypePrincipal canister_id);

  // Installs a module from chunks in the chunk store of store_canister (by
  // default the target itself), in the order of chunk_hashes;
  // wasm_module_hash is the sha256 of the whole module.
  static IC_Task<IC_CallResult<void>>
  install_chunked_code(CandidTypePrincipal target_canister, IC_InstallMode mode,
                       std::vector<IC_Blob> chunk_hashes,
                       IC_Blob wasm_module_hash, IC_Blob arg = {},
                       std::optional<CandidTypePrincipal> store_canister = {});

  // Removes the code and the state of a canister this canister controls.
  static IC_Task<IC_CallResult<void>>
  uninstall_code(CandidTypePrincipal canister_id);

  // The custom section "icp:public <name>" of any canister's module, or
  // "icp:private <name>" of a canister this canister controls.
  static IC_Task<IC_CallResult<IC_Blob>>
  canister_metadata(CandidTypePrincipal canister_id, std::string name);

  // The public information of any canister: its controllers, its module hash
  // and its most recent changes: num_requested_changes of them (at most 20),
  // none when it is not set.
  static IC_Task<IC_CallResult<IC_CanisterInfo>>
  canister_info(CandidTypePrincipal canister_id,
                std::optional<uint64_t> num_requested_changes = {});

  // Snapshots of a canister this canister controls. A snapshot is taken of,
  // and loaded into, a stopped canister. take_canister_snapshot can replace
  // an earlier one, and uninstall the code once the snapshot is taken.
  static IC_Task<IC_CallResult<IC_Snapshot>>
  take_canister_snapshot(CandidTypePrincipal canister_id,
                         std::optional<IC_Blob> replace_snapshot = {},
                         std::optional<bool> uninstall_code = {});
  static IC_Task<IC_CallResult<void>>
  load_canister_snapshot(CandidTypePrincipal canister_id, IC_Blob snapshot_id);
  static IC_Task<IC_CallResult<std::vector<IC_Snapshot>>>
  list_canister_snapshots(CandidTypePrincipal canister_id);
  static IC_Task<IC_CallResult<void>>
  delete_canister_snapshot(CandidTypePrincipal canister_id,
                           IC_Blob snapshot_id);

  // Reading a snapshot, e.g. to download it (with snapshot_visibility, also
  // a snapshot of a canister this canister does not control), and uploading
  // one, e.g. to restore it elsewhere: first the metadata, which gives the
  // new snapshot's id, then the data in chunks.
  static IC_Task<IC_CallResult<IC_SnapshotMetadata>>
  read_canister_snapshot_metadata(CandidTypePrincipal canister_id,
                                  IC_Blob snapshot_id);
  static IC_Task<IC_CallResult<IC_Blob>>
  read_canister_snapshot_data(CandidTypePrincipal canister_id,
                              IC_Blob snapshot_id, IC_SnapshotData kind);
  static IC_Task<IC_CallResult<IC_Blob>> upload_canister_snapshot_metadata(
      CandidTypePrincipal canister_id, IC_SnapshotMetadata metadata,
      std::optional<IC_Blob> replace_snapshot = {});
  static IC_Task<IC_CallResult<void>>
  upload_canister_snapshot_data(CandidTypePrincipal canister_id,
                                IC_Blob snapshot_id, IC_SnapshotData kind,
                                IC_Blob chunk);

  // Threshold ECDSA (secp256k1 or secp256r1): the private key is spread over the nodes of
  // a signing subnet and never exists in one place. The public key gives a
  // Bitcoin or Ethereum address that this canister controls; sign_with_ecdsa
  // signs a 32-byte hash, e.g. of a transaction.
  static IC_Task<IC_CallResult<IC_PublicKey>>
  ecdsa_public_key(IC_EcdsaKeyId key, std::vector<IC_Blob> derivation_path,
                   std::optional<CandidTypePrincipal> canister_id = {});
  static IC_Task<IC_CallResult<IC_Blob>>
  sign_with_ecdsa(IC_Blob message_hash, std::vector<IC_Blob> derivation_path,
                  IC_EcdsaKeyId key);

  // Threshold Schnorr: bip340secp256k1 for Bitcoin Taproot (with an optional
  // bip341_merkle_root_hash), ed25519 for e.g. Solana. Like ECDSA, no private
  // key exists in one place.
  static IC_Task<IC_CallResult<IC_PublicKey>>
  schnorr_public_key(IC_SchnorrKeyId key, std::vector<IC_Blob> derivation_path,
                     std::optional<CandidTypePrincipal> canister_id = {});
  static IC_Task<IC_CallResult<IC_Blob>>
  sign_with_schnorr(IC_Blob message, std::vector<IC_Blob> derivation_path,
                    IC_SchnorrKeyId key,
                    std::optional<IC_Blob> bip341_merkle_root_hash = {});

  // vetKD (vetKeys): derives a secret key for an input, encrypted to the
  // client's transport public key, so no node sees it. For end-to-end
  // encryption, with this canister deciding who gets which key.
  static IC_Task<IC_CallResult<IC_Blob>>
  vetkd_public_key(IC_VetKdKeyId key, IC_Blob context,
                   std::optional<CandidTypePrincipal> canister_id = {});
  static IC_Task<IC_CallResult<IC_Blob>>
  vetkd_derive_key(IC_Blob input, IC_Blob context, IC_Blob transport_public_key,
                   IC_VetKdKeyId key);

  // An HTTPS request to a server on the internet, e.g. a price feed
  static IC_Task<IC_CallResult<IC_HttpOutcallResponse>>
  http_request(IC_HttpOutcallRequest request);
}; // docs end: ic_management

#endif // __cpp_impl_coroutine
