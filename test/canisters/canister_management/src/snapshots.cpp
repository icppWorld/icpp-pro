// The snapshot helpers of IC_Management: take, load, list and delete a
// snapshot, and read and upload one.

#include "my_canister.h"

#include <algorithm>
#include <optional>
#include <string>
#include <vector>

#include "ic_api.h"
#include "test_helpers.h"

namespace {
void reply_ok_or_error(IC_API &ic_api, const IC_CallResult<void> &r) {
  ic_api.to_wire(CandidTypeText{r ? std::string("ok") : error_text(r.error())});
}

std::string snapshot_text(const IC_Snapshot &s) {
  return hex_text(s.id) + ":" + std::to_string(s.total_size);
}

std::string global_text(const IC_SnapshotGlobal &g) {
  switch (g.kind) {
  case IC_SnapshotGlobal::Kind::I64:
    return "i64:" + std::to_string(g.i64);
  case IC_SnapshotGlobal::Kind::F32:
    return "f32:" + std::to_string(g.f32);
  case IC_SnapshotGlobal::Kind::F64:
    return "f64:" + std::to_string(g.f64);
  case IC_SnapshotGlobal::Kind::V128:
    return "v128:" + nat_text(g.v128);
  case IC_SnapshotGlobal::Kind::I32:
    break;
  }
  return "i32:" + std::to_string(g.i32);
}

std::string metadata_text(const IC_SnapshotMetadata &m) {
  std::string globals;
  for (size_t i = 0; i < m.globals.size(); ++i) {
    globals += (i ? "," : "") + global_text(m.globals[i]);
  }
  std::string timer = "none";
  if (m.global_timer) {
    timer = m.global_timer->active
                ? "active@" + std::to_string(m.global_timer->timestamp)
                : "inactive";
  }
  std::string hook = "none";
  if (m.on_low_wasm_memory_hook_status) {
    switch (*m.on_low_wasm_memory_hook_status) {
    case IC_LowWasmMemoryHookStatus::Ready:
      hook = "ready";
      break;
    case IC_LowWasmMemoryHookStatus::Executed:
      hook = "executed";
      break;
    case IC_LowWasmMemoryHookStatus::ConditionNotSatisfied:
      hook = "condition_not_satisfied";
      break;
    }
  }
  return std::string("source=") +
         (m.source == IC_ChangeDetails::SnapshotSource::MetadataUpload
              ? "metadata_upload"
              : "taken_from_canister") +
         "; taken_at=" + std::to_string(m.taken_at_timestamp) +
         "; wasm_module_size=" + std::to_string(m.wasm_module_size) +
         "; globals=" + globals +
         "; wasm_memory_size=" + std::to_string(m.wasm_memory_size) +
         "; stable_memory_size=" + std::to_string(m.stable_memory_size) +
         "; chunks=" + std::to_string(m.wasm_chunk_store.size()) +
         "; canister_version=" + std::to_string(m.canister_version) +
         "; certified_data=" + hex_text(m.certified_data) + "; timer=" + timer +
         "; hook=" + hook;
}

IC_SnapshotData data_kind(const std::string &name, uint64_t offset,
                          uint64_t size, const IC_Blob &hash) {
  IC_SnapshotData kind;
  kind.offset = offset;
  kind.size = size;
  kind.hash = hash;
  if (name == "wasm_memory") kind.kind = IC_SnapshotData::Kind::WasmMemory;
  else if (name == "stable_memory")
    kind.kind = IC_SnapshotData::Kind::StableMemory;
  else if (name == "wasm_chunk") kind.kind = IC_SnapshotData::Kind::WasmChunk;
  return kind;
}

// The bytes of one part of a snapshot, read in chunks of at most 1 MiB
IC_Task<IC_CallResult<IC_Blob>> read_all(const CandidTypePrincipal &canister,
                                         const IC_Blob &snapshot_id,
                                         IC_SnapshotData::Kind kind,
                                         uint64_t size) {
  IC_Blob all;
  for (uint64_t offset = 0; offset < size;) {
    IC_SnapshotData part;
    part.kind = kind;
    part.offset = offset;
    part.size = std::min<uint64_t>(1024 * 1024, size - offset);
    IC_CallResult<IC_Blob> r =
        co_await IC_Management::read_canister_snapshot_data(canister,
                                                            snapshot_id, part);
    if (!r) co_return r.error();
    all.insert(all.end(), r->begin(), r->end());
    offset += part.size;
  }
  co_return all;
}
} // namespace

// '(canister_id, replace_snapshot, uninstall_code)' -> '("<id>:<size>")';
// replace_snapshot is a vec of 0 or 1 snapshot id
IC_Async take_snapshot_child_async() {
  IC_API ic_api(CanisterUpdate{"take_snapshot_child"}, false);
  std::string canister_id;
  std::vector<IC_Blob> replace;
  std::optional<bool> uninstall_code;
  CandidArgs args;
  args.append(CandidTypePrincipal{&canister_id});
  args.append(CandidTypeVecBlob{&replace});
  args.append(CandidTypeOptBool{&uninstall_code});
  ic_api.from_wire(args);
  std::optional<IC_Blob> replace_snapshot;
  if (!replace.empty()) replace_snapshot = replace[0];
  IC_CallResult<IC_Snapshot> r = co_await IC_Management::take_canister_snapshot(
      CandidTypePrincipal{canister_id}, replace_snapshot, uninstall_code);
  ic_api.to_wire(CandidTypeText{r ? snapshot_text(*r) : error_text(r.error())});
}
void take_snapshot_child() { take_snapshot_child_async(); }

// '(canister_id, snapshot_id)' -> '("ok")'
IC_Async load_snapshot_child_async() {
  IC_API ic_api(CanisterUpdate{"load_snapshot_child"}, false);
  std::string canister_id;
  IC_Blob snapshot_id;
  CandidArgs args;
  args.append(CandidTypePrincipal{&canister_id});
  args.append(CandidTypeVecNat8{&snapshot_id});
  ic_api.from_wire(args);
  IC_CallResult<void> r = co_await IC_Management::load_canister_snapshot(
      CandidTypePrincipal{canister_id}, snapshot_id);
  reply_ok_or_error(ic_api, r);
}
void load_snapshot_child() { load_snapshot_child_async(); }

// '(canister_id)' -> '("<id>:<size>,...")'
IC_Async list_snapshots_child_async() {
  IC_API ic_api(CanisterUpdate{"list_snapshots_child"}, false);
  std::string canister_id;
  ic_api.from_wire(CandidTypePrincipal{&canister_id});
  IC_CallResult<std::vector<IC_Snapshot>> r =
      co_await IC_Management::list_canister_snapshots(
          CandidTypePrincipal{canister_id});
  if (!r) {
    ic_api.to_wire(CandidTypeText{error_text(r.error())});
    co_return;
  }
  std::string text;
  for (size_t i = 0; i < r->size(); ++i) {
    text += (i ? "," : "") + snapshot_text((*r)[i]);
  }
  ic_api.to_wire(CandidTypeText{text});
}
void list_snapshots_child() { list_snapshots_child_async(); }

// '(canister_id, snapshot_id)' -> '("ok")'
IC_Async delete_snapshot_child_async() {
  IC_API ic_api(CanisterUpdate{"delete_snapshot_child"}, false);
  std::string canister_id;
  IC_Blob snapshot_id;
  CandidArgs args;
  args.append(CandidTypePrincipal{&canister_id});
  args.append(CandidTypeVecNat8{&snapshot_id});
  ic_api.from_wire(args);
  IC_CallResult<void> r = co_await IC_Management::delete_canister_snapshot(
      CandidTypePrincipal{canister_id}, snapshot_id);
  reply_ok_or_error(ic_api, r);
}
void delete_snapshot_child() { delete_snapshot_child_async(); }

// '(canister_id, snapshot_id)' -> '("source=..; ...")'
IC_Async snapshot_metadata_child_async() {
  IC_API ic_api(CanisterUpdate{"snapshot_metadata_child"}, false);
  std::string canister_id;
  IC_Blob snapshot_id;
  CandidArgs args;
  args.append(CandidTypePrincipal{&canister_id});
  args.append(CandidTypeVecNat8{&snapshot_id});
  ic_api.from_wire(args);
  IC_CallResult<IC_SnapshotMetadata> r =
      co_await IC_Management::read_canister_snapshot_metadata(
          CandidTypePrincipal{canister_id}, snapshot_id);
  ic_api.to_wire(CandidTypeText{r ? metadata_text(*r) : error_text(r.error())});
}
void snapshot_metadata_child() { snapshot_metadata_child_async(); }

// '(canister_id, snapshot_id, kind, offset, size, hash)' -> '("<hex>")'
IC_Async read_snapshot_data_child_async() {
  IC_API ic_api(CanisterUpdate{"read_snapshot_data_child"}, false);
  std::string canister_id, kind;
  IC_Blob snapshot_id, hash;
  uint64_t offset{0}, size{0};
  CandidArgs args;
  args.append(CandidTypePrincipal{&canister_id});
  args.append(CandidTypeVecNat8{&snapshot_id});
  args.append(CandidTypeText{&kind});
  args.append(CandidTypeNat64{&offset});
  args.append(CandidTypeNat64{&size});
  args.append(CandidTypeVecNat8{&hash});
  ic_api.from_wire(args);
  IC_CallResult<IC_Blob> r =
      co_await IC_Management::read_canister_snapshot_data(
          CandidTypePrincipal{canister_id}, snapshot_id,
          data_kind(kind, offset, size, hash));
  ic_api.to_wire(CandidTypeText{r ? hex_text(*r) : error_text(r.error())});
}
void read_snapshot_data_child() { read_snapshot_data_child_async(); }

// '(canister_id, snapshot_id, kind, offset, chunk)' -> '("ok")'
IC_Async upload_snapshot_data_child_async() {
  IC_API ic_api(CanisterUpdate{"upload_snapshot_data_child"}, false);
  std::string canister_id, kind;
  IC_Blob snapshot_id, chunk;
  uint64_t offset{0};
  CandidArgs args;
  args.append(CandidTypePrincipal{&canister_id});
  args.append(CandidTypeVecNat8{&snapshot_id});
  args.append(CandidTypeText{&kind});
  args.append(CandidTypeNat64{&offset});
  args.append(CandidTypeVecNat8{&chunk});
  ic_api.from_wire(args);
  IC_CallResult<void> r = co_await IC_Management::upload_canister_snapshot_data(
      CandidTypePrincipal{canister_id}, snapshot_id,
      data_kind(kind, offset, 0, {}), chunk);
  reply_ok_or_error(ic_api, r);
}
void upload_snapshot_data_child() { upload_snapshot_data_child_async(); }

// '(canister_id, snapshot_id)' -> '("<the new snapshot's id>")': reads the
// snapshot's metadata and data, and uploads them as a new snapshot of the
// same canister
IC_Async copy_snapshot_child_async() {
  IC_API ic_api(CanisterUpdate{"copy_snapshot_child"}, false);
  std::string canister_id;
  IC_Blob snapshot_id;
  CandidArgs args;
  args.append(CandidTypePrincipal{&canister_id});
  args.append(CandidTypeVecNat8{&snapshot_id});
  ic_api.from_wire(args);
  CandidTypePrincipal canister{canister_id};

  IC_CallResult<IC_SnapshotMetadata> md =
      co_await IC_Management::read_canister_snapshot_metadata(canister,
                                                              snapshot_id);
  if (!md) {
    ic_api.to_wire(CandidTypeText{error_text(md.error())});
    co_return;
  }
  IC_CallResult<IC_Blob> new_id =
      co_await IC_Management::upload_canister_snapshot_metadata(canister, *md);
  if (!new_id) {
    ic_api.to_wire(CandidTypeText{error_text(new_id.error())});
    co_return;
  }
  struct Part {
    IC_SnapshotData::Kind kind;
    uint64_t size;
  };
  for (const Part &part :
       {Part{IC_SnapshotData::Kind::WasmModule, md->wasm_module_size},
        Part{IC_SnapshotData::Kind::WasmMemory, md->wasm_memory_size},
        Part{IC_SnapshotData::Kind::StableMemory, md->stable_memory_size}}) {
    IC_CallResult<IC_Blob> bytes =
        co_await read_all(canister, snapshot_id, part.kind, part.size);
    if (!bytes) {
      ic_api.to_wire(CandidTypeText{error_text(bytes.error())});
      co_return;
    }
    for (uint64_t offset = 0; offset < bytes->size();) {
      uint64_t n = std::min<uint64_t>(1024 * 1024, bytes->size() - offset);
      IC_SnapshotData upload;
      upload.kind = part.kind;
      upload.offset = offset;
      IC_CallResult<void> r =
          co_await IC_Management::upload_canister_snapshot_data(
              canister, *new_id, upload,
              IC_Blob(bytes->begin() + offset, bytes->begin() + offset + n));
      if (!r) {
        ic_api.to_wire(CandidTypeText{error_text(r.error())});
        co_return;
      }
      offset += n;
    }
  }
  ic_api.to_wire(CandidTypeText{hex_text(*new_id)});
}
void copy_snapshot_child() { copy_snapshot_child_async(); }
