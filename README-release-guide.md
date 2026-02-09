# icpp-pro & icpp-candid release

This guide explains how to release icpp-pro & icpp-candid.

We release icpp-pro via PyPI. Please read the [Packaging Python Projects](https://packaging.python.org/en/latest/tutorials/packaging-projects) guide before continuing.

## Setup

Follow ALL the steps of the [Contributors Guide](README-contributors-guide.md), so you have a fully functioning environment where all the tests pass.

## Checkout `main` Branch
In the `icpp-pro` & `icpp-candid` repositories, checkout the `main` branch.

## Turn off Debug

In `icpp-candid/src/icpp_candid/candid/candid_debug_config.h` set flag to `false`.

## Select the version number

We follow the version naming convention for python packages ([pep-0440](https://peps.python.org/pep-0440/)).

**icpp-pro and icpp-candid always share the same version number.** Even if only one package has changes, both get bumped to the new version. This avoids complicated dependency mapping.

### Version Number

- In both repos, update the version number in: `src/icpp/version.py`
- In icpp-pro repo, update the icpp-candid dependency in `pyproject.toml`:
  ```toml
  dependencies = [
    "icpp-candid>=x.y.z",    # Update to match the new version
  ```
- Push `icpp-candid` first, because the cicd of `icpp-pro` depends on that
- Push `icpp-pro`

Push the version.py directly to `main` branch in GitHub, with commit message `x.y.z` (or `x.y.zrci`, eg. `1.0.0rc1` for release candidates)

- Make sure the CI/CD passes

If all OK, then proceed with publishing the package.

## Build icpp-candid

```bash
conda activate icpp-pro

cd icpp-candid
git checkout main

make pypi-build
```

## Build icpp-pro

```bash
conda activate icpp-pro
git checkout main

make pypi-build
```

## Test icpp-pro & icpp-candid

```bash
# clean out previous installations
rm -rf ~/.icpp

# create a brand new python environment
conda create --name test python=3.12  # Test for python=3.9-3.12
conda activate test

# Start fresh
pip cache purge

# Create a release-test folder as a sibling of icpp-pro and copy the wheels
mkdir -p ../release-test/dist
cp dist/* ../release-test/dist/
cp icpp-candid/dist/* ../release-test/dist/
cd ../release-test

# Option 1: Everything from local wheels
pip install dist/icpp_candid-x.y.z-py3-none-any.whl
pip install dist/icpp_pro-x.y.z-py3-none-any.whl

# Option 2: icpp-pro from local wheel & icpp-candid from testPyPI
pip install --extra-index-url https://test.pypi.org/simple/ --no-cache dist/icpp_pro-x.y.z-py3-none-any.whl

# Option 3: icpp-pro from local wheel & icpp-candid from PyPI
pip install --no-cache dist/icpp_pro-x.y.z-py3-none-any.whl

# Verify packages installed
pip list
icpp-candid        x.y.z
icpp-pro           x.y.z

# test: basics
icpp
icpp --help
icpp --version

# test: `greet` project
icpp init
cd greet
dfx identity use default
./demo.sh
./demo-c++17.sh

# Cleanup
cd ../..
rm -rf release-test
conda deactivate
conda remove --name test --all
```

## Upload icpp-candid to TestPyPI & PyPI

```bash
cd icpp-candid
# check content of .pypirc

make testpypi-upload
make pypi-upload
```

## Upload icpp-pro to TestPyPI & PyPI

```bash
cd icpp-pro
# check content of .pypirc

make testpypi-upload
make pypi-upload
```

## Follow up steps

- Redeploy greet canister to IC
  ```
  cd src/icpp/canisters/greet
  icpp build-wasm
  dfx deploy --ic
  ```
- Release new docs
- Announcement in OpenChat
- Update icpp-demos
  - Redeploy canister of api-reference
    ```
    cd canisters/api_reference
    icpp build-wasm
    dfx deploy --ic
    ```
