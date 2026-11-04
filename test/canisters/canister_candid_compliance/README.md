# canister_candid_compliance

Runs the test suite of the
[Candid Interface Specification](https://github.com/dfinity/candid/blob/master/spec/Candid.md)
against icpp-candid's decoder and textual value parser, together with the
Candid tests of the Rust and Motoko CDKs, ported to the same assert format.
Every assert must pass.

| Source            | Version                                                                                         |
|-------------------|-------------------------------------------------------------------------------------------------|
| `suite/official/` | dfinity/candid `test/` at `572c80a867b935c1de5c428366e6762aa1cc6c71` (2026-10-21)               |
| Specification     | spec 0.1.8, `spec/Candid.md` at the same commit                                                 |
| `suite/rust/`     | ported from dfinity/candid `rust/candid/tests`, `rust/candid_parser/tests`, same commit         |
| `suite/motoko/`   | ported from dfinity/motoko `test/run-deser`, `test/run-drun/idl-*.mo` at `01aee06` (2026-09-16) |

| Suite file                               | Asserts  |
|------------------------------------------|----------|
| official/construct.test.did              | 169      |
| official/overshoot.test.did              | 10       |
| official/prim.test.did                   | 168      |
| official/reference.test.did              | 50       |
| official/spacebomb.test.did              | 17       |
| official/subtypes.test.did               | 58       |
| rust/compatibility.test.did              | 28       |
| rust/header_len.test.did                 | 10       |
| rust/number.test.did                     | 41       |
| rust/parse_value.test.did                | 18       |
| rust/serde.test.did                      | 263      |
| rust/service_compatibility.test.did      | 35       |
| rust/skip_optional_queue.test.did        | 3        |
| rust/subtype.test.did                    | 2        |
| rust/type_diagnostics.test.did           | 5        |
| rust/type_table_alloc.test.did           | 3        |
| rust/types.test.did                      | 6        |
| rust/value.test.did                      | 16       |
| motoko/idl-any-stable.test.did           | 1        |
| motoko/idl-any.test.did                  | 29       |
| motoko/idl-bad.test.did                  | 1        |
| motoko/idl-buf-size-bug.test.did         | 1        |
| motoko/idl-composite.test.did            | 4        |
| motoko/idl-field-escape.test.did         | 1        |
| motoko/idl-float32.test.did              | 1        |
| motoko/idl-func.test.did                 | 1        |
| motoko/idl-large-principal.test.did      | 6        |
| motoko/idl-nary.test.did                 | 7        |
| motoko/idl-nat-int.test.did              | 4        |
| motoko/idl-option.test.did               | 3        |
| motoko/idl-pair.test.did                 | 1        |
| motoko/idl-principal.test.did            | 1        |
| motoko/idl-record.test.did               | 5        |
| motoko/idl-shorthand.test.did            | 1        |
| motoko/idl-spacebomb.test.did            | 12       |
| motoko/idl-sub-ho-neg.test.did           | 19       |
| motoko/idl-sub-ho.test.did               | 11       |
| motoko/idl-sub-opt-any-record.test.did   | 3        |
| motoko/idl-sub-opt-any.test.did          | 5        |
| motoko/idl-sub-rec.test.did              | 11       |
| motoko/idl-sub-variant.test.did          | 2        |
| motoko/idl-sub.test.did                  | 15       |
| motoko/idl-tuple.test.did                | 6        |
| motoko/idl-unexpected-variant.test.did   | 1        |
| motoko/idl-variant.test.did              | 1        |
| motoko/idl-vector.test.did               | 1        |
| motoko/run-deser.test.did                | 10       |
| **official total**                       | **472**  |
| **rust total**                           | **430**  |
| **motoko total**                         | **164**  |
| **total**                                | **1066** |

## How it works

`scripts/candid_compliance/generate.py` turns each `.test.did` file into
`src/suite_*.cpp`: one `ComplianceRunner::check` call per assert, with the
expected types built as a `CandidTypeGraph`. The generated files are checked
in. After a change to a `.test.did` file, regenerate them from the repo root:

```bash
python scripts/candid_compliance/generate.py
```

To update the suite, copy upstream's `test/*.test.did` into `suite/official/`,
update the commit above, and regenerate.

The ports in `suite/rust/` and `suite/motoko/` say at the top of each file
which tests they come from, and which of those are not ported, and why.

## Running it

```bash
# natively: exits non-zero if any assert fails
icpp build-native && icpp run-native

# on a local replica: one pytest per suite file
icpp build-wasm
icp network start --background
icp deploy --environment local --yes --identity $ICPP_PRO_TEST_IDENTITY
pytest --network=local
```
