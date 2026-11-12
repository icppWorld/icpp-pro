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

## Contributing

- Set up with the [Contributors Guide](README-contributors-guide.md). The
  [Feature Guide](README-feature-guide.md) describes how a change is planned,
  tested and released.
- Work on a branch and open a pull request into `main`. Every pull request
  needs a manual approval from the maintainer before it merges.
- Before each push to a pull request:
  - a local CodeRabbit review of the branch against `main`
    (`coderabbit review --base main --agent`);
  - a code review and a security review of the branch;
  - `make all-static` and the tests of the affected canisters pass.
- CodeRabbit on GitHub also reviews every push to a pull request. Each of its
  findings gets a fix, or a reply saying why not.
- The pull request lists each review's findings and what was done about them.
- A change to `CLAUDE.md` or to the skills in `.claude/skills/` also gets a
  prompt audit (`/claude-api prompt-audit`).
- A release gets two deeper reviews on top: see the
  [release guide](README-release-guide.md#deep-reviews-of-the-release).

## Community

Please join our [C++ community on OpenChat](https://oc.app/community/cklkv-3aaaa-aaaar-ar7uq-cai/?ref=6e3y2-4yaaa-aaaaf-araya-cai)

