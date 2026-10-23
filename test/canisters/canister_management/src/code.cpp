// The code helpers of IC_Management: install_code, the chunk store,
// install_chunked_code, uninstall_code, canister_metadata and canister_info.

#include "my_canister.h"

#include <optional>
#include <string>
#include <vector>

#include "ic_api.h"
#include "test_helpers.h"

namespace {
// "install", "reinstall", "upgrade", "upgrade_skip" (skip_pre_upgrade) or
// "upgrade_keep" (keep the wasm memory)
IC_InstallMode mode_for(const std::string &name) {
  IC_InstallMode mode;
  if (name == "reinstall") mode.kind = IC_InstallMode::Kind::Reinstall;
  else if (name.rfind("upgrade", 0) == 0) {
    mode.kind = IC_InstallMode::Kind::Upgrade;
    if (name == "upgrade_skip") mode.skip_pre_upgrade = true;
    if (name == "upgrade_keep")
      mode.wasm_memory_persistence = IC_WasmMemoryPersistence::Keep;
  }
  return mode;
}

const char *mode_text(IC_InstallMode::Kind kind) {
  switch (kind) {
  case IC_InstallMode::Kind::Reinstall:
    return "reinstall";
  case IC_InstallMode::Kind::Upgrade:
    return "upgrade";
  case IC_InstallMode::Kind::Install:
    break;
  }
  return "install";
}

// One change of canister_info, e.g. "code_deployment(upgrade,<hash>)/user"
std::string change_text(const IC_Change &c) {
  const IC_ChangeDetails &d = c.details;
  std::string s;
  switch (d.kind) {
  case IC_ChangeDetails::Kind::Creation:
    s = "creation(" + std::to_string(d.controllers.size()) +
        (d.environment_variables_hash ? ",env" : "") + ")";
    break;
  case IC_ChangeDetails::Kind::CodeUninstall:
    s = "code_uninstall";
    break;
  case IC_ChangeDetails::Kind::CodeDeployment:
    s = std::string("code_deployment(") + mode_text(d.mode) + "," +
        hex_text(d.module_hash).substr(0, 8) + ")";
    break;
  case IC_ChangeDetails::Kind::LoadSnapshot:
    s = "load_snapshot(" + hex_text(d.snapshot_id) + "," +
        std::to_string(d.snapshot_canister_version) + "," +
        (d.snapshot_source == IC_ChangeDetails::SnapshotSource::MetadataUpload
             ? "metadata_upload"
             : "taken_from_canister") +
        ")";
    break;
  case IC_ChangeDetails::Kind::ControllersChange:
    s = "controllers_change(" + std::to_string(d.controllers.size()) + ")";
    break;
  case IC_ChangeDetails::Kind::SettingsChange:
    s = "settings_change(" +
        (d.settings_controllers ? std::to_string(d.settings_controllers->size())
                                : std::string("-")) +
        (d.environment_variables_hash ? ",env" : "") + ")";
    break;
  case IC_ChangeDetails::Kind::RenameCanister:
    s = "rename_canister(" + d.renamed_canister_id + "->" +
        d.rename_to_canister_id + "," + std::to_string(d.rename_to_version) +
        ")";
    break;
  }
  if (c.origin.kind == IC_ChangeOrigin::Kind::FromUser) {
    s += "/user";
  } else {
    s += "/canister";
    if (c.origin.canister_version)
      s += "@" + std::to_string(*c.origin.canister_version);
  }
  return s;
}

void reply_ok_or_error(IC_API &ic_api, const IC_CallResult<void> &r) {
  ic_api.to_wire(CandidTypeText{r ? std::string("ok") : error_text(r.error())});
}
} // namespace

// '(canister_id, mode, wasm_module, arg)' -> '("ok")' or '("rej:...")'
IC_Async install_child_async() {
  IC_API ic_api(CanisterUpdate{"install_child"}, false);
  std::string canister_id, mode;
  IC_Blob wasm_module, arg;
  CandidArgs args;
  args.append(CandidTypePrincipal{&canister_id});
  args.append(CandidTypeText{&mode});
  args.append(CandidTypeVecNat8{&wasm_module});
  args.append(CandidTypeVecNat8{&arg});
  ic_api.from_wire(args);
  IC_CallResult<void> r = co_await IC_Management::install_code(
      CandidTypePrincipal{canister_id}, mode_for(mode), wasm_module, arg);
  reply_ok_or_error(ic_api, r);
}
void install_child() { install_child_async(); }

// '(canister_id, chunk)' -> '("<hash as hex>")' or '("rej:...")'
IC_Async upload_chunk_child_async() {
  IC_API ic_api(CanisterUpdate{"upload_chunk_child"}, false);
  std::string canister_id;
  IC_Blob chunk;
  CandidArgs args;
  args.append(CandidTypePrincipal{&canister_id});
  args.append(CandidTypeVecNat8{&chunk});
  ic_api.from_wire(args);
  IC_CallResult<IC_Blob> r = co_await IC_Management::upload_chunk(
      CandidTypePrincipal{canister_id}, chunk);
  ic_api.to_wire(CandidTypeText{r ? hex_text(*r) : error_text(r.error())});
}
void upload_chunk_child() { upload_chunk_child_async(); }

// '(canister_id, size)' -> upload_chunk of size bytes, made here: an argument
// of 1 MiB does not fit on a command line
IC_Async upload_big_chunk_child_async() {
  IC_API ic_api(CanisterUpdate{"upload_big_chunk_child"}, false);
  std::string canister_id;
  uint64_t size{0};
  CandidArgs args;
  args.append(CandidTypePrincipal{&canister_id});
  args.append(CandidTypeNat64{&size});
  ic_api.from_wire(args);
  IC_CallResult<IC_Blob> r = co_await IC_Management::upload_chunk(
      CandidTypePrincipal{canister_id}, IC_Blob(size, 'x'));
  ic_api.to_wire(CandidTypeText{r ? hex_text(*r) : error_text(r.error())});
}
void upload_big_chunk_child() { upload_big_chunk_child_async(); }

// '(canister_id)' -> '("<hash>,<hash>,...")' or '("rej:...")'
IC_Async stored_chunks_child_async() {
  IC_API ic_api(CanisterUpdate{"stored_chunks_child"}, false);
  std::string canister_id;
  ic_api.from_wire(CandidTypePrincipal{&canister_id});
  IC_CallResult<std::vector<IC_Blob>> r =
      co_await IC_Management::stored_chunks(CandidTypePrincipal{canister_id});
  if (!r) {
    ic_api.to_wire(CandidTypeText{error_text(r.error())});
    co_return;
  }
  std::string text;
  for (size_t i = 0; i < r->size(); ++i) {
    text += (i ? "," : "") + hex_text((*r)[i]);
  }
  ic_api.to_wire(CandidTypeText{text});
}
void stored_chunks_child() { stored_chunks_child_async(); }

// '(canister_id)' -> '("ok")' or '("rej:...")'
IC_Async clear_chunks_child_async() {
  IC_API ic_api(CanisterUpdate{"clear_chunks_child"}, false);
  std::string canister_id;
  ic_api.from_wire(CandidTypePrincipal{&canister_id});
  IC_CallResult<void> r = co_await IC_Management::clear_chunk_store(
      CandidTypePrincipal{canister_id});
  reply_ok_or_error(ic_api, r);
}
void clear_chunks_child() { clear_chunks_child_async(); }

// '(canister_id, mode, chunk_hashes, wasm_module_hash, store_canister)'
//   -> '("ok")' or '("rej:...")'
IC_Async install_chunked_child_async() {
  IC_API ic_api(CanisterUpdate{"install_chunked_child"}, false);
  std::string canister_id, mode;
  std::vector<IC_Blob> chunk_hashes;
  IC_Blob wasm_module_hash;
  std::optional<std::string> store_canister;
  CandidArgs args;
  args.append(CandidTypePrincipal{&canister_id});
  args.append(CandidTypeText{&mode});
  args.append(CandidTypeVecBlob{&chunk_hashes});
  args.append(CandidTypeVecNat8{&wasm_module_hash});
  args.append(CandidTypeOptPrincipal{&store_canister});
  ic_api.from_wire(args);
  std::optional<CandidTypePrincipal> store;
  if (store_canister) store = CandidTypePrincipal{*store_canister};
  IC_CallResult<void> r = co_await IC_Management::install_chunked_code(
      CandidTypePrincipal{canister_id}, mode_for(mode), chunk_hashes,
      wasm_module_hash, {}, store);
  reply_ok_or_error(ic_api, r);
}
void install_chunked_child() { install_chunked_child_async(); }

// '(canister_id)' -> '("ok")' or '("rej:...")'
IC_Async uninstall_child_async() {
  IC_API ic_api(CanisterUpdate{"uninstall_child"}, false);
  std::string canister_id;
  ic_api.from_wire(CandidTypePrincipal{&canister_id});
  IC_CallResult<void> r =
      co_await IC_Management::uninstall_code(CandidTypePrincipal{canister_id});
  reply_ok_or_error(ic_api, r);
}
void uninstall_child() { uninstall_child_async(); }

// '(canister_id, name)' -> '("value=<text>")' or '("rej:...")'
IC_Async metadata_child_async() {
  IC_API ic_api(CanisterUpdate{"metadata_child"}, false);
  std::string canister_id, name;
  CandidArgs args;
  args.append(CandidTypePrincipal{&canister_id});
  args.append(CandidTypeText{&name});
  ic_api.from_wire(args);
  IC_CallResult<IC_Blob> r = co_await IC_Management::canister_metadata(
      CandidTypePrincipal{canister_id}, name);
  ic_api.to_wire(CandidTypeText{r ? "value=" + std::string(r->begin(), r->end())
                                  : error_text(r.error())});
}
void metadata_child() { metadata_child_async(); }

// '(canister_id, num_requested_changes)'
//   -> '("total=..; controllers=..; module_hash=..; changes=..")'
IC_Async info_child_async() {
  IC_API ic_api(CanisterUpdate{"info_child"}, false);
  std::string canister_id;
  std::optional<uint64_t> num_requested_changes;
  CandidArgs args;
  args.append(CandidTypePrincipal{&canister_id});
  args.append(CandidTypeOptNat64{&num_requested_changes});
  ic_api.from_wire(args);
  IC_CallResult<IC_CanisterInfo> r = co_await IC_Management::canister_info(
      CandidTypePrincipal{canister_id}, num_requested_changes);
  if (!r) {
    ic_api.to_wire(CandidTypeText{error_text(r.error())});
    co_return;
  }
  std::string changes;
  for (size_t i = 0; i < r->recent_changes.size(); ++i) {
    changes += (i ? " " : "") + change_text(r->recent_changes[i]);
  }
  ic_api.to_wire(CandidTypeText{
      "total=" + std::to_string(r->total_num_changes) + "; controllers=" +
      std::to_string(r->controllers.size()) + "; module_hash=" +
      (r->module_hash ? hex_text(*r->module_hash) : std::string("none")) +
      "; changes=" + changes});
}
void info_child() { info_child_async(); }
