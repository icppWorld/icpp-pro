[![icpp-pro](https://github.com/icppWorld/icpp-pro/actions/workflows/cicd-ubuntu.yml/badge.svg)](https://github.com/icppWorld/icpp-pro/actions/workflows/cicd-ubuntu.yml)
[![icpp-pro](https://github.com/icppWorld/icpp-pro/actions/workflows/cicd-mac.yml/badge.svg)](https://github.com/icppWorld/icpp-pro/actions/workflows/cicd-mac.yml)

# C++ Canister Development Kit

icpp-pro is a cdk to develop C++ smart contracts for the  [Internet Computer](https://internetcomputer.org/).

Its Candid library is 100% compliant with the [Candid Interface Specification](https://github.com/dfinity/candid/blob/572c80a/spec/Candid.md) (spec 0.1.8, dfinity/candid@572c80a), within its [limits](https://icpp-pro.onicai.com/candid-compliance.html#limits): it decodes every message of the spec, and encodes every value, recursive types included, to the bytes of the reference implementation. All 472 asserts of the official suite pass, and all 1190 with the Candid tests of the Rust and Motoko CDKs. See [Candid compliance](https://icpp-pro.onicai.com/candid-compliance.html).

# In the wild

Check out these real world on-chain AI examples build with icpp-pro:

- [funnai.onicai.com](https://funnai.onicai.com/) ✨
- [icgpt.onicai.com](https://icgpt.onicai.com/) ✨
- [IConfucius](https://github.com/onicai/IConfucius) ✨
- [llama_cpp_canister](https://github.com/onicai/llama_cpp_canister) ✨

--- 

## Installation

```bash
pip install icpp-pro
```

## Documentation

See the [icpp-pro documentation](https://icpp-pro.onicai.com).

## Source Code

The source code is open source and available at:

- [icpp-pro](https://github.com/icppWorld/icpp-pro)
- [icpp-candid](https://github.com/icppWorld/icpp-candid)

## Community

Please join our [C++ community on OpenChat](https://oc.app/community/cklkv-3aaaa-aaaar-ar7uq-cai/?ref=6e3y2-4yaaa-aaaaf-araya-cai)

