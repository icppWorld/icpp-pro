# canister_candid_compliance

Runs the test suite of the
[Candid Interface Specification](https://github.com/dfinity/candid/blob/master/spec/Candid.md)
against icpp-candid's decoder and textual value parser. Every assert must pass.

| Source              | Version                                                                                   |
|---------------------|-------------------------------------------------------------------------------------------|
| `suite/official/`   | dfinity/candid `test/` at `572c80a867b935c1de5c428366e6762aa1cc6c71` (2026-10-21)         |
| Specification       | spec 0.1.8, `spec/Candid.md` at the same commit                                          |

| Suite file                   | Asserts |
|------------------------------|---------|
| official/prim.test.did       | 168     |
| official/construct.test.did  | 169     |
| official/reference.test.did  | 50      |
| official/subtypes.test.did   | 58      |
| official/spacebomb.test.did  | 17      |
| official/overshoot.test.did  | 10      |
| **total**                    | **472** |

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
