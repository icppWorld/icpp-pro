// Candid performance benchmarks: instructions per decode and per encode.
//
// The email-canister cases use its own code: decode_smtp_request decodes with
// CandidDecoder as its smtp/smtp_request.cpp does, decode_mail_send and the
// encoders use the typed API as its api/*.cpp do. The values are built from
// n with the same formulas as test/test_apis.py, which checks the hashes.

#include "my_canister.h"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "candid_decoder.h"
#include "candid_serialize.h"
#include "ic_api.h"

namespace {

using PC = IC_API::PerformanceCounterType;

// ---------------------------------------------------------------------------
// The reply: instructions, and the size and FNV-1a hash of the bytes
struct Fnv1a {
  uint64_t h = 0xcbf29ce484222325ULL;
  uint64_t size = 0;
  void add(const uint8_t *p, size_t n) {
    for (size_t i = 0; i < n; ++i) {
      h ^= p[i];
      h *= 0x100000001b3ULL;
    }
    size += n;
  }
  void add(const std::string &s) {
    add(reinterpret_cast<const uint8_t *>(s.data()), s.size());
  }
  void add(const std::vector<uint8_t> &v) { add(v.data(), v.size()); }
  void add_u64(uint64_t v) {
    uint8_t b[8];
    for (int i = 0; i < 8; ++i)
      b[i] = static_cast<uint8_t>(v >> (8 * i));
    add(b, 8);
  }
};

void reply(IC_API &ic_api, uint64_t instructions, const Fnv1a &f) {
  CandidTypeRecord r;
  r.append("instructions", CandidTypeNat64{instructions});
  r.append("size", CandidTypeNat64{f.size});
  r.append("fnv1a", CandidTypeNat64{f.h});
  ic_api.to_wire(r);
}

// Serializes one argument, as IC_API::to_wire does, and counts it with the
// building of the value: `build` returns the CandidType to send
template <typename Build> void encode_and_reply(IC_API &ic_api, Build build) {
  const uint64_t start = IC_API::performance_counter(PC::Instructions);
  CandidArgs args;
  args.append(build());
  const VecBytes B = CandidSerialize(args).get_B();
  const uint64_t instructions =
      IC_API::performance_counter(PC::Instructions) - start;
  Fnv1a f;
  f.add(B.vec_uint8_t());
  reply(ic_api, instructions, f);
}

uint64_t read_n(IC_API &ic_api) {
  uint64_t n = 0;
  ic_api.from_wire(CandidTypeNat64{&n});
  return n;
}

// ---------------------------------------------------------------------------
// The values, as test_apis.py builds them
uint8_t blob_byte(uint64_t j) { return static_cast<uint8_t>(j * 31 + 7); }
char text_char(uint64_t j) { return static_cast<char>('a' + j % 26); }
uint64_t nat64_at(uint64_t j) { return j * 0x9E3779B97F4A7C15ULL; }

std::vector<uint8_t> make_blob(uint64_t n) {
  std::vector<uint8_t> v(n);
  for (uint64_t j = 0; j < n; ++j)
    v[j] = blob_byte(j);
  return v;
}

std::string make_text(uint64_t n) {
  std::string s(n, ' ');
  for (uint64_t j = 0; j < n; ++j)
    s[j] = text_char(j);
  return s;
}

// The email-canister's store::Summary
struct Summary {
  uint64_t uid, state, flags, modseq, received_ns, sent_at;
  std::string envelope_from, subject, sender;
  uint64_t attachments;
  std::string preview;
  uint64_t raw_bytes, folder;
  std::string sender_address;
};

Summary make_summary(uint64_t i) {
  const std::string address = "sender" + std::to_string(i) + "@example.com";
  return Summary{i,
                 i % 3,
                 i * 7,
                 1000 + i,
                 1795000000000000000ULL + i,
                 1794000000000000000ULL + i,
                 address,
                 "Subject number " + std::to_string(i) + " of the benchmark",
                 "Sender " + std::to_string(i),
                 i % 4,
                 std::string(100, 'p') + std::to_string(i),
                 5000 + i,
                 i % 5,
                 address};
}

// As the email-canister's summary_value (api/mailbox_api.cpp)
CandidType summary_value(const Summary &s) {
  CandidTypeRecord r;
  r.append("uid", CandidTypeNat64{s.uid});
  r.append("state", CandidTypeNat64{s.state});
  r.append("flags", CandidTypeNat64{s.flags});
  r.append("modseq", CandidTypeNat64{s.modseq});
  r.append("received_ns", CandidTypeNat64{s.received_ns});
  r.append("sent_at", CandidTypeNat64{s.sent_at});
  r.append("envelope_from", CandidTypeText{s.envelope_from});
  r.append("subject", CandidTypeText{s.subject});
  r.append("sender", CandidTypeText{s.sender});
  r.append("attachments", CandidTypeNat64{s.attachments});
  r.append("preview", CandidTypeText{s.preview});
  r.append("raw_bytes", CandidTypeNat64{s.raw_bytes});
  r.append("folder", CandidTypeNat64{s.folder});
  r.append("sender_address", CandidTypeText{s.sender_address});
  return r;
}

CandidType seen_value(const std::string &principal,
                      const std::optional<std::string> &name, uint64_t at_ns) {
  CandidTypeRecord r;
  r.append("principal", CandidTypePrincipal{principal});
  r.append("name", CandidTypeOptText{name});
  r.append("at_ns", CandidTypeNat64{at_ns});
  return r;
}

struct Record10 {
  uint64_t f0 = 0, f1 = 0, f2 = 0, f3 = 0, f4 = 0;
  std::string f5, f6, f7, f8, f9;
};

Record10 make_record10(uint64_t i) {
  return Record10{i,
                  i * 2,
                  i * 3,
                  i + 1000000,
                  i ^ 0xffff,
                  "name" + std::to_string(i),
                  "value" + std::to_string(i),
                  "t",
                  "",
                  std::string(20, 'x')};
}

CandidTypeRecord record10_type(Record10 &r) {
  CandidTypeRecord t;
  t.append("f0", CandidTypeNat64{&r.f0});
  t.append("f1", CandidTypeNat64{&r.f1});
  t.append("f2", CandidTypeNat64{&r.f2});
  t.append("f3", CandidTypeNat64{&r.f3});
  t.append("f4", CandidTypeNat64{&r.f4});
  t.append("f5", CandidTypeText{&r.f5});
  t.append("f6", CandidTypeText{&r.f6});
  t.append("f7", CandidTypeText{&r.f7});
  t.append("f8", CandidTypeText{&r.f8});
  t.append("f9", CandidTypeText{&r.f9});
  return t;
}

CandidType record10_value(const Record10 &r) {
  CandidTypeRecord t;
  t.append("f0", CandidTypeNat64{r.f0});
  t.append("f1", CandidTypeNat64{r.f1});
  t.append("f2", CandidTypeNat64{r.f2});
  t.append("f3", CandidTypeNat64{r.f3});
  t.append("f4", CandidTypeNat64{r.f4});
  t.append("f5", CandidTypeText{r.f5});
  t.append("f6", CandidTypeText{r.f6});
  t.append("f7", CandidTypeText{r.f7});
  t.append("f8", CandidTypeText{r.f8});
  t.append("f9", CandidTypeText{r.f9});
  return t;
}

void hash_record10(Fnv1a &f, const Record10 &r) {
  f.add_u64(r.f0);
  f.add_u64(r.f1);
  f.add_u64(r.f2);
  f.add_u64(r.f3);
  f.add_u64(r.f4);
  f.add(r.f5);
  f.add(r.f6);
  f.add(r.f7);
  f.add(r.f8);
  f.add(r.f9);
}

// ---------------------------------------------------------------------------
// decode_smtp_request: the email-canister's smtp/smtp_request.cpp
uint32_t id(const char *name) { return CandidTypeGraph::idl_hash(name); }

int add_record(CandidTypeGraph &g, std::vector<CandidGraphField> fields) {
  std::sort(fields.begin(), fields.end(),
            [](const CandidGraphField &a, const CandidGraphField &b) {
              return a.id < b.id;
            });
  CandidGraphNode node;
  node.kind = CandidKind::Record;
  node.fields = std::move(fields);
  return g.add(std::move(node));
}

int add_wrapper(CandidTypeGraph &g, CandidKind kind, int inner) {
  CandidGraphNode node;
  node.kind = kind;
  node.inner = inner;
  return g.add(std::move(node));
}

int smtp_request_type(CandidTypeGraph &g) {
  const int text = CandidTypeGraph::prim(CandidKind::Text);
  const int nat8 = CandidTypeGraph::prim(CandidKind::Nat8);

  const int header = add_record(g, {{id("name"), text}, {id("value"), text}});
  const int message =
      add_record(g, {{id("headers"), add_wrapper(g, CandidKind::Vec, header)},
                     {id("body"), add_wrapper(g, CandidKind::Vec, nat8)}});
  const int address = add_record(g, {{id("user"), text}, {id("domain"), text}});
  const int envelope =
      add_record(g, {{id("from"), address},
                     {id("to"), add_wrapper(g, CandidKind::Vec, address)}});

  const int vec_text = add_wrapper(g, CandidKind::Vec, text);
  return add_record(
      g, {{id("message"), add_wrapper(g, CandidKind::Opt, message)},
          {id("envelope"), add_wrapper(g, CandidKind::Opt, envelope)},
          {id("gateway_flags"), add_wrapper(g, CandidKind::Opt, vec_text)},
          {id("message_id"), add_wrapper(g, CandidKind::Opt, text)}});
}

const CandidValue *opt_value(const CandidValue &record, const char *name) {
  const CandidValue *opt = record.field(id(name));
  if (opt == nullptr || opt->items.empty()) return nullptr;
  return &opt->items[0];
}

struct SmtpMessage {
  std::vector<std::pair<std::string, std::string>> headers;
  std::vector<uint8_t> body;
};

} // namespace

// ---------------------------------------------------------------------------
// Decoding

void decode_smtp_request() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  SmtpMessage out;
  {
    const VecBytes bytes = ic_api.get_B_in();
    CandidTypeGraph graph;
    const std::vector<int> args{smtp_request_type(graph)};
    std::vector<CandidValue> values;
    std::string err;
    if (!CandidDecoder::decode(bytes.vec_uint8_t().data(),
                               bytes.vec_uint8_t().size(), graph, args, values,
                               err)) {
      IC_API::trap(err);
    }
    if (const CandidValue *message = opt_value(values[0], "message")) {
      const CandidValue *headers = message->field(id("headers"));
      const CandidValue *body = message->field(id("body"));
      if (headers == nullptr || body == nullptr) IC_API::trap("bad message");
      out.headers.resize(headers->vec_size());
      for (size_t i = 0; i < out.headers.size(); ++i) {
        const CandidValue *name = headers->at(i).field(id("name"));
        const CandidValue *value = headers->at(i).field(id("value"));
        if (name == nullptr || value == nullptr) IC_API::trap("bad header");
        out.headers[i] = {name->text, value->text};
      }
      out.body = body->bytes;
    }
  }
  const uint64_t instructions = IC_API::performance_counter(PC::Instructions);
  Fnv1a f;
  f.add(out.body);
  for (const auto &h : out.headers) {
    f.add(h.first);
    f.add(h.second);
  }
  reply(ic_api, instructions, f);
}

// As the email-canister's mail_send (api/outbox_api.cpp)
void decode_mail_send() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  struct Attachment {
    std::string filename;
    std::vector<uint8_t> content;
  };
  std::string client_id, subject, html_body;
  std::vector<std::string> to, cc, bcc;
  std::vector<Attachment> attachments;
  Attachment attachment;
  CandidTypeRecord in;
  in.append("client_id", CandidTypeText{&client_id});
  in.append("to", CandidTypeVecText{&to});
  in.append("cc", CandidTypeVecText{&cc});
  in.append("bcc", CandidTypeVecText{&bcc});
  in.append("subject", CandidTypeText{&subject});
  in.append("html_body", CandidTypeText{&html_body});
  in.append("attachments",
            CandidTypeVecOf(
                [&]() -> CandidType {
                  attachment = {};
                  CandidTypeRecord r;
                  r.append("filename", CandidTypeText{&attachment.filename});
                  r.append("content", CandidTypeVecNat8{&attachment.content});
                  return r;
                },
                [&]() { attachments.push_back(attachment); }));
  ic_api.from_wire(in);
  const uint64_t instructions = IC_API::performance_counter(PC::Instructions);
  Fnv1a f;
  f.add(client_id);
  for (const auto *v : {&to, &cc, &bcc})
    for (const auto &s : *v)
      f.add(s);
  f.add(subject);
  f.add(html_body);
  for (const auto &a : attachments) {
    f.add(a.filename);
    f.add(a.content);
  }
  reply(ic_api, instructions, f);
}

void decode_blob() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  std::vector<uint8_t> v;
  ic_api.from_wire(CandidTypeVecNat8{&v});
  const uint64_t instructions = IC_API::performance_counter(PC::Instructions);
  Fnv1a f;
  f.add(v);
  reply(ic_api, instructions, f);
}

void decode_text() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  std::string s;
  ic_api.from_wire(CandidTypeText{&s});
  const uint64_t instructions = IC_API::performance_counter(PC::Instructions);
  Fnv1a f;
  f.add(s);
  reply(ic_api, instructions, f);
}

void decode_vec_nat64() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  std::vector<uint64_t> v;
  ic_api.from_wire(CandidTypeVecNat64{&v});
  const uint64_t instructions = IC_API::performance_counter(PC::Instructions);
  Fnv1a f;
  for (uint64_t x : v)
    f.add_u64(x);
  reply(ic_api, instructions, f);
}

void decode_vec_record10() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  std::vector<Record10> v;
  Record10 r;
  ic_api.from_wire(CandidTypeVecOf(
      [&]() -> CandidType {
        r = {};
        return record10_type(r);
      },
      [&]() { v.push_back(r); }));
  const uint64_t instructions = IC_API::performance_counter(PC::Instructions);
  Fnv1a f;
  for (const auto &x : v)
    hash_record10(f, x);
  reply(ic_api, instructions, f);
}

// ---------------------------------------------------------------------------
// Encoding

// As the email-canister's reply_page (api/mailbox_api.cpp): a Page of n
// summaries, with cursor n
void encode_mail_list() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  const uint64_t n = read_n(ic_api);
  std::vector<Summary> items;
  for (uint64_t i = 0; i < n; ++i)
    items.push_back(make_summary(i));
  encode_and_reply(ic_api, [&]() -> CandidType {
    std::vector<CandidType> elements;
    for (const Summary &s : items)
      elements.push_back(summary_value(s));
    std::optional<uint64_t> cursor = n;
    CandidTypeRecord r;
    r.append("items", CandidTypeVecOf(summary_value(Summary{}), elements));
    r.append("cursor", CandidTypeOptNat64{cursor});
    return r;
  });
}

// As the email-canister's mail_get: a FullMessage with a raw message of n
// bytes, seen by two
void encode_mail_get() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  const uint64_t n = read_n(ic_api);
  const Summary summary = make_summary(n);
  const std::vector<uint8_t> raw = make_blob(n);
  encode_and_reply(ic_api, [&]() -> CandidType {
    std::vector<CandidType> seen;
    seen.push_back(seen_value("2vxsx-fae", std::string("Alice"), 1));
    seen.push_back(seen_value("aaaaa-aa", std::nullopt, 2));
    CandidTypeRecord r;
    r.append("summary", summary_value(summary));
    r.append("raw", CandidTypeVecNat8{raw});
    r.append("seen_by", CandidTypeVecOf(seen_value("", std::nullopt, 0), seen));
    return r;
  });
}

// As the email-canister's mail_export_chunk: a chunk of n bytes, moved in
void encode_mail_export_chunk() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  const uint64_t n = read_n(ic_api);
  std::vector<uint8_t> data = make_blob(n);
  encode_and_reply(ic_api, [&]() -> CandidType {
    CandidTypeRecord r;
    r.append("data", CandidTypeVecNat8{std::move(data)});
    r.append("size", CandidTypeNat64{n});
    r.append("started_ns", CandidTypeNat64{uint64_t{1795000000000000000ULL}});
    return r;
  });
}

void encode_blob() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  const std::vector<uint8_t> v = make_blob(read_n(ic_api));
  encode_and_reply(ic_api,
                   [&]() -> CandidType { return CandidTypeVecNat8{v}; });
}

void encode_text() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  const std::string s = make_text(read_n(ic_api));
  encode_and_reply(ic_api, [&]() -> CandidType { return CandidTypeText{s}; });
}

void encode_vec_nat64() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  const uint64_t n = read_n(ic_api);
  std::vector<uint64_t> v(n);
  for (uint64_t j = 0; j < n; ++j)
    v[j] = nat64_at(j);
  encode_and_reply(ic_api,
                   [&]() -> CandidType { return CandidTypeVecNat64{v}; });
}

void encode_vec_record10() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  const uint64_t n = read_n(ic_api);
  std::vector<Record10> items;
  for (uint64_t i = 0; i < n; ++i)
    items.push_back(make_record10(i));
  encode_and_reply(ic_api, [&]() -> CandidType {
    std::vector<CandidType> elements;
    for (const Record10 &r : items)
      elements.push_back(record10_value(r));
    return CandidTypeVecOf(record10_value(Record10{}), elements);
  });
}

// A record of n nat64 fields, f0 = 0 ... f<n-1> = n-1
void encode_record() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  const uint64_t n = read_n(ic_api);
  std::vector<std::string> names;
  for (uint64_t i = 0; i < n; ++i)
    names.push_back("f" + std::to_string(i));
  encode_and_reply(ic_api, [&]() -> CandidType {
    CandidTypeRecord r;
    for (uint64_t i = 0; i < n; ++i)
      r.append(names[i], CandidTypeNat64{i});
    return r;
  });
}

// A small reply, as the email-canister's smtp_reply_err: variant { Err =
// record { code = n; message = "..." } }
void encode_small() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  const uint64_t n = read_n(ic_api);
  encode_and_reply(ic_api, [&]() -> CandidType {
    CandidTypeRecord error;
    error.append("code", CandidTypeNat64{n});
    error.append("message", CandidTypeText{std::string("mailbox not found")});
    return CandidTypeVariant{"Err", error};
  });
}
