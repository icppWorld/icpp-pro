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

- Update the version number in both repos: `icpp-pro/src/icpp/version.py` and
  `icpp-candid/src/icpp_candid/version.py`
- In icpp-pro repo, update the icpp-candid dependency in `pyproject.toml`:
  ```toml
  dependencies = [
    "icpp-candid==x.y.z",    # Update to match the new version
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
conda create --name test python=3.11  # Test for python=3.11-3.14
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

# Option 2: icpp-pro from local wheel & icpp-candid from PyPI
# (only works once icpp-candid x.y.z is published - see "Tag & publish")
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
# The demo scripts create & use their own `greet-testing` identity, and never
# touch your machine-wide active identity. Nothing to set up here.

# Capture the output - see "Verifying the demo scripts" below for why
sh ./demo.sh       2>&1 | tee /tmp/release-demo.log
sh ./demo-c++17.sh 2>&1 | tee /tmp/release-demo17.log

# Cleanup
cd ../..
rm -rf release-test
conda deactivate
conda remove --name test --all
```

### Verifying the demo scripts

**A clean exit from `demo.sh` does NOT mean the tests passed.** The demo scripts
are plain `#!/bin/sh` with no `set -e`, so they run to completion even when a step
fails, and their exit code only reflects the **last** command in the script (the
native `mockic.exe` run) - not `pytest`, which ran much earlier. Scrolling past and
seeing the script "finish" proves nothing.

So check the pytest section explicitly. In each log, find:

```
============================= test session starts ==============================
collected 10 items

test/test_apis.py ..........                                             [100%]

============================== 10 passed in 4.11s ==============================
```

Every character must be a `.`; an `F` or `E` is a failure. Then scan both logs for
anything that slipped through:

```bash
grep -inE "[0-9]+ (failed|error)|FAILED |AssertionError|Traceback|no tests ran|collected 0" \
     /tmp/release-demo.log /tmp/release-demo17.log
```

The only acceptable hits are `Error: network 'local' is not running` (the harmless
`icp network stop` at the top of the script, before anything is running) and the
native suite's own `- Failed   : 0`.

### Definitive check: run pytest directly

Better than reading logs - run `pytest` yourself and look at its exit code:

```bash
# from: release-test/greet
export ICPP_PRO_TEST_IDENTITY=greet-testing   # created by demo.sh

icp network stop; rm -rf .icp/cache; icp network start --background
icp deploy --environment local --yes --identity $ICPP_PRO_TEST_IDENTITY

pytest --network=local -q; echo "pytest exit code = $?"
#  -> 10 passed / exit code = 0
```

Deploy with the same identity the tests run as - `test__greet_0_auth_ok` asserts
on the caller's principal.

Note `echo $?` must be its **own** statement. Piping pytest into `tail`/`grep` and
then echoing `$?` reports the exit code of the *pipe*, not of pytest.

### Negative control (recommended)

A green suite only means something if the tests can actually fail. Confirm they are
really talking to the deployed canister:

```bash
icp network stop
pytest --network=local -q; echo "pytest exit code = $?"
#  -> "The local network is not up" / "no tests ran" / exit code = 2

# And that a missing identity is refused rather than guessed:
env -u ICPP_PRO_TEST_IDENTITY pytest --network=local -q
#  -> "ERROR: no test identity configured." / exit code = 2
```

If that still reports passes, the tests are not reaching the canister and the green
run above was meaningless.

## Upgrade test (backward-compatibility gate)

A canister built & deployed with the **previous** icpp-pro release from PyPI
must upgrade in place to the release candidate, with all state surviving — no
data-structure migration, no source changes (see README-feature-guide.md).

```bash
# from: icpp-pro, in the dev conda env (the "dev" side of the upgrade)
make upgrade-test
#  -> "✅ upgrade test passed: released ➜ dev upgrade is clean"
```

## Verify the sibling repos

Mandatory before tagging: icpp-docs build, icpp-demos test suite and
llama_cpp_canister native tests, all against the dev install.

```bash
# from: icpp-pro
make check-sibling-pins
make siblings-verify-api
```

The llama docker wasm leg (`make siblings-verify-full`, hours) runs
after the llama_cpp_canister pin bump. Before the release, llama's docker
build can only install the previous icpp-pro from PyPI (its `docker-compose.yml`
pins the version), so it would not exercise the release at all.

## Deep reviews of the release

Every PR of the release already had the reviews of the README's
[Contributing](README.md#contributing) section. Before a release, everything
since the previous release gets two deeper reviews on top. There is no release
PR (the version bump goes straight to `main`), so each repo gets a review-only
PR of its changes since the previous tag. Open it after the version bump is
pushed, in icpp-candid and in icpp-pro:

```bash
# from: icpp-candid, then from: icpp-pro, each on an up-to-date main
PREV=v7.1.0rc2   # the previous release tag
git push origin "$PREV^{commit}:refs/heads/release-base/$PREV"   # ^{commit}: works for annotated tags too
gh pr create --draft --base "release-base/$PREV" --head main --title "Release review: changes since $PREV" --body "Review-only PR for the deep reviews of the release. Never merge it: it is closed after the release."
```

Then, on each review PR:

- Ask the maintainer to run `/code-review ultra <PR number>`: it is billed, and
  only the maintainer can start it.
- Run `/claude-security` "Scan changes" at high effort, from that repo on
  `main`, after a `git fetch`:
  `/claude-security scan changes --base origin/release-base/$PREV --effort high`.
- CodeRabbit on GitHub reviews the PR as well.

Fix their findings like any other review's: on a feature branch, through a PR
that merges before the release. Those fixes change the commits to tag, so do
these reviews before the fuzz run below, which must run on the exact commits
that are tagged.

After the release, close each review PR and delete its base branch. Do not use
`gh pr close --delete-branch` here: the PR's head branch is `main`.

```bash
gh pr close <PR number>
git push origin --delete "release-base/$PREV"
```

## Fuzz the release candidate

Mandatory before tagging. On every push, CI fuzzes the Candid decoder & encoder
for a minute per target. A release gets an hour per target, about 3.5 hours in
total, from the manually triggered `fuzz` workflow:

```bash
# Or: GitHub > Actions > fuzz > Run workflow
gh workflow run fuzz.yml -R icppWorld/icpp-pro --ref main -f seconds=3600
gh run watch -R icppWorld/icpp-pro $(gh run list -R icppWorld/icpp-pro --workflow fuzz.yml --limit 1 --json databaseId --jq '.[0].databaseId')
```

It fuzzes icpp-pro's `--ref` and icpp-candid's `main`. To fuzz another
icpp-candid branch, add `-f icpp-candid-ref=<branch>`.

A crash blocks the release. Download the input from the run's `fuzz-crashes`
artifact, fix the bug in icpp-candid and keep the input in
`test/fuzz/regressions/`. Put the run length, the result and the two commits
printed by the run's `versions` step in the release notes.

## Tag & publish

**Prerequisite: do not tag until the `fuzz` workflow has passed with
`seconds=3600` on the exact icpp-pro and icpp-candid commits you are about to
tag.** It is started by hand and nothing starts it for you; see
[Fuzz the release candidate](#fuzz-the-release-candidate). The commits are in
the run's `versions` step. To find the run:

```bash
gh run list -R icppWorld/icpp-pro --workflow fuzz.yml --branch main --status success --limit 1
```

Publishing is **CI-only**. Pushing a `vX.Y.Z` tag runs that repo's
`.github/workflows/release.yml`, which builds the distributions and uploads
them to PyPI with [trusted publishing](https://docs.pypi.org/trusted-publishers/)
(OIDC). There is no API token, no `.pypirc`, and nothing is ever uploaded from
a laptop.

Tag **icpp-candid first**: icpp-pro pins `icpp-candid==X.Y.Z`, and icpp-pro's
release workflow refuses to publish until that exact version is on PyPI.

```bash
# 1. icpp-candid
cd icpp-candid
git checkout main && git pull
git tag vx.y.z
git push origin vx.y.z
```

Watch the run to completion — **a red leg means nothing was published**:

```bash
gh run watch -R icppWorld/icpp-candid $(gh run list -R icppWorld/icpp-candid --workflow release.yml --limit 1 --json databaseId --jq '.[0].databaseId')
```

Confirm it is really on PyPI before continuing:

```bash
pip download icpp-candid==x.y.z --no-deps -d /tmp/candid-check
```

```bash
# 2. icpp-pro
cd ..
git checkout main && git pull
git tag vx.y.z
git push origin vx.y.z
```

Watch that run the same way (`-R icppWorld/icpp-pro`).

The tag must point at a commit whose `version.py` already says `x.y.z` — both
workflows assert this and fail the release on a mismatch, rather than quietly
republishing the previous version.

### One-time setup (maintainer)

Required once per project, before the first tag-triggered release can publish
(and again if a publisher ever has to be re-created).

On PyPI (pypi.org → project → Manage → Publishing → add a GitHub publisher):

| PyPI project  | Owner       | Repository    | Workflow name | Environment |
|---------------|-------------|---------------|---------------|-------------|
| `icpp-candid` | `icppWorld` | `icpp-candid` | `release.yml` | `pypi`      |
| `icpp-pro`    | `icppWorld` | `icpp-pro`    | `release.yml` | `pypi`      |

On GitHub: each repo needs an environment named `pypi`. It scopes the OIDC claim,
and its required reviewer (`icppWorld`) approves each publish: the release run
waits at the `publish` job until the reviewer approves it, under "Review
deployments" on the run's page.

## Follow up steps

- Redeploy greet canister to IC, as the identity that controls it on mainnet.
  The machine-wide active identity is usually `anonymous`, which cannot deploy,
  so the name comes from `.icp-identity` at the repo root (git-ignored, one
  line, per developer - see README-contributors-guide.md):
  ```
  make greet-deploy-ic
  ```
- Bump the sibling version pins to the released version, then verify with
  `make check-sibling-pins` until green:
  - `icpp-demos/requirements.txt` — `icpp-pro>=X.Y.Z`
  - `llama_cpp_canister/requirements.txt` — `icpp-pro==X.Y.Z` **and**
    `llama_cpp_canister/docker/docker-compose.yml` — both the
    `icpp: &icpp "X.Y.Z"` anchor and the
    `name: &base_name "llama-cpp-canister-build:icpp-X.Y.Z"` literal.
    The wasm hash changes with an icpp-pro bump: follow the llama repo's own
    release process (`.claude/skills/llama_cpp_canister-release`).
    With the pin bumped, run the docker wasm leg against the published release:
    `make siblings-verify-full` (from icpp-pro; hours).
- Release new docs (the `icpp-docs` repo):
  - Update the version label in `mkdocs.yml` nav, `docs/index.md`, and add the
    release to `docs/release-notes.md`
  - `make site-build` and `make icp-deploy` (from icpp-docs)
- Announcement in OpenChat
- Update icpp-demos
  - Redeploy canister of api-reference, as the identity in icpp-demos'
    own `.icp-identity` (from icpp-demos):
    ```
    make api-reference-deploy-ic
    ```
