// Ingest a state with update self-calls, then ask about it with a query, or
// with a composite query that calls queries of the canister itself. It mimics
// running a model in llama_cpp_canister: ingest the state with updates, then
// ask a question about it with a query.

#include "canister_ingest.h"

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include "ic_api.h"

namespace {
// The ingested state: one message per line
constexpr const char *STATE_PATH = "state.txt";

std::vector<std::string> read_state() {
  std::vector<std::string> lines;
  std::ifstream file(STATE_PATH);
  std::string line;
  while (std::getline(file, line))
    lines.push_back(line);
  return lines;
}

uint64_t instructions() {
  return IC_API::performance_counter(
      IC_API::PerformanceCounterType::Instructions);
}

std::string error_text(const IC_CallError &error) {
  return std::to_string(static_cast<uint32_t>(error.reject_code)) + ":" +
         error.reject_message;
}

// The answer to a question about the state: how many messages mention a word
std::string answer(const std::string &word) {
  std::vector<std::string> lines = read_state();
  size_t mentions = 0;
  for (const std::string &line : lines)
    if (line.find(word) != std::string::npos) ++mentions;
  return std::to_string(lines.size()) + " messages, " +
         std::to_string(mentions) + " mention " + word +
         "; instructions: " + std::to_string(instructions());
}
} // namespace

// ---------------------------------------------------------------------------
// Ingest

void reset() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  ic_api.from_wire();
  std::remove(STATE_PATH);
  ic_api.to_wire();
}

IC_Async ingest_async() {
  IC_API ic_api(CanisterUpdate{"ingest"}, false);
  std::vector<std::string> messages;
  ic_api.from_wire(CandidTypeVecText{&messages});

  CandidTypePrincipal self = ic_api.get_canister_self();
  for (const std::string &message : messages) {
    IC_CallResult<IC_CallReply> r =
        co_await IC_Call::bounded_wait(self, "store")
            .with_arg(CandidTypeText{message});
    if (!r) IC_API::trap("ingest: store failed: " + error_text(r.error()));
  }

  std::ifstream file(STATE_PATH, std::ios::binary | std::ios::ate);
  ic_api.to_wire(CandidTypeNat64{file ? (uint64_t)file.tellg() : 0});
}
void ingest() { ingest_async(); }

void store() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  if (ic_api.get_caller().get_text() != ic_api.get_canister_self().get_text())
    IC_API::trap("store: only the canister itself may call store");
  std::string message;
  ic_api.from_wire(CandidTypeText{&message});

  std::ofstream file(STATE_PATH, std::ios::app);
  file << message << "\n";
  file.close();
  if (!file) IC_API::trap(std::string("store: failed to write ") + STATE_PATH);
  ic_api.to_wire();
}

// ---------------------------------------------------------------------------
// Ask

void ask() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  std::string word;
  ic_api.from_wire(CandidTypeText{&word});
  ic_api.to_wire(CandidTypeText{answer(word)});
}

IC_Async ask_cq_async() {
  IC_API ic_api(CanisterCompositeQuery{"ask_cq"}, false);
  std::string word;
  ic_api.from_wire(CandidTypeText{&word});

  IC_CallResult<IC_CallReply> r =
      co_await IC_Call::bounded_wait(ic_api.get_canister_self(), "ask")
          .with_arg(CandidTypeText{word});
  std::string text;
  if (r) r->decode(CandidTypeText{&text});
  ic_api.to_wire(
      CandidTypeText{r ? "via ask: " + text : error_text(r.error())});
}
void ask_cq() { ask_cq_async(); }

// ---------------------------------------------------------------------------
// The instruction budget

void burn_q() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  uint64_t n{0};
  ic_api.from_wire(CandidTypeNat64{&n});
  // x = a * x + c (mod 2^64), in blocks between two reads of the counter
  volatile uint64_t x = 1;
  while (instructions() < n)
    for (int i = 0; i < 65536; ++i)
      x = 6364136223846793005ULL * x + 1442695040888963407ULL;
  ic_api.to_wire(CandidTypeNat64{instructions()});
}

IC_Async cq_burn_async() {
  IC_API ic_api(CanisterCompositeQuery{"cq_burn"}, false);
  uint32_t parts{0};
  uint64_t n{0};
  CandidArgs args;
  args.append(CandidTypeNat32{&parts});
  args.append(CandidTypeNat64{&n});
  ic_api.from_wire(args);

  CandidTypePrincipal self = ic_api.get_canister_self();
  uint32_t done = 0;
  std::string burned, error;
  for (; done < parts; ++done) {
    IC_CallResult<IC_CallReply> r =
        co_await IC_Call::bounded_wait(self, "burn_q")
            .with_arg(CandidTypeNat64{n});
    if (!r) {
      error = "; error: " + error_text(r.error());
      break;
    }
    uint64_t used{0};
    r->decode(CandidTypeNat64{&used});
    burned += (done ? ", " : "") + std::to_string(used);
  }
  ic_api.to_wire(CandidTypeText{"parts: " + std::to_string(done) + " of " +
                                std::to_string(parts) + "; burn_q: " + burned +
                                "; own: " + std::to_string(instructions()) +
                                error});
}
void cq_burn() { cq_burn_async(); }
