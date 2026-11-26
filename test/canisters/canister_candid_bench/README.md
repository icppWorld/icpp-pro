# canister_candid_bench

Measures how many instructions icpp-candid takes to decode and to encode
Candid, on wasm, and guards against slowdowns in CI.

Each method replies with a `Result`:

| Field          | Decode methods                                                      | Encode methods                                 |
|----------------|---------------------------------------------------------------------|------------------------------------------------|
| `instructions` | the whole message up to the end of decoding, argument copy included | building the Candid types and serializing them |
| `size`         | the number of payload bytes decoded                                 | the number of bytes of the encoded message     |
| `fnv1a`        | the 64-bit FNV-1a hash of those payload bytes                       | the 64-bit FNV-1a hash of the encoded message  |

The cases follow the email-canister's most common calls, with its own code:

| Case                | What is measured                                          | Sizes                                       |
|---------------------|-----------------------------------------------------------|---------------------------------------------|
| `smtp_request`      | decoding a `SmtpRequest` with `CandidDecoder`             | 20 headers, a body of 1 KB, 100 KB, 1.9 MB  |
| `mail_send`         | decoding a `SendRequest` with the typed API               | no attachments, 1 x 1 MB, 3 x 600 KB        |
| `mail_list`         | encoding a `Page` of 14-field `Summary` records           | 50 and 500 summaries                        |
| `mail_get`          | encoding a `FullMessage`                                  | a raw message of 100 KB and 1.9 MB          |
| `mail_export_chunk` | encoding a chunk, moved into `CandidTypeVecNat8`          | 1.9 MB                                      |

And general ones: `blob`, `text` and `vec nat64` of 1 KB, 100 KB and 1.9 MB
each way, a `vec` of 10-field records with 10, 1,000 and 10,000 elements each
way, records of 10 and 50 fields, and a small `variant` reply.

## How the tests check

`test/test_apis.py`, against the deployed canister:

- A decode case sends the bytes of `didc encode`, and checks the hash of what
  the canister decoded.
- An encode case has `didc encode` the same value, and checks that the
  canister's bytes are the same, byte for byte.
- Every case compares its instructions with `test/baseline.json`, within
  `TOLERANCE` (10%):
  - more than 10% above its baseline fails;
  - more than 10% below it passes, with a warning to update the baseline.

The results of the last run are in `build/bench-results.json`.

After a change that makes a case faster, record the new baseline, as the test
identity the canister was deployed with, and commit it with the change:

```bash
ICPP_CANDID_BENCH_UPDATE=1 pytest --network=local --identity icpp-pro-testing
```

An update still fails, and does not record, a case that got more than 10%
slower. To record a slowdown on purpose, use `ICPP_CANDID_BENCH_UPDATE=force`.

The replica decides what a system call costs, so `icp.yaml` pins the network
launcher. Bump its `version` on purpose, and record the baseline again in the
same change.

The launcher for macOS on ARM charges less for some system calls than the ones
for Linux and Intel macOS, which give the same counts. On macOS ARM the cases
check only what they decode and encode, and the baseline is not recorded there.

Natively, `ic0.performance_counter` counts nanoseconds, so `native/main.cpp`
checks only that each method replies the right size.

The baseline was recorded with icpp-pro 7.2.0 in development (wasi-sdk 25), on
icp-cli-network-launcher v16.1.0-2026-11-20-03-19.
