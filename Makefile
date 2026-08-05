SHELL := /bin/bash

# Disable built-in rules and variables
MAKEFLAGS += --no-builtin-rules
MAKEFLAGS += --no-builtin-variables

NETWORK := local

###########################################################################
# OS we're running on
ifeq ($(OS),Windows_NT)
	detected_OS := Windows
else
	detected_OS := $(shell sh -c 'uname 2>/dev/null || echo Unknown')
endif

ifeq ($(detected_OS),Darwin)	  # Mac OS X  (Intel)
	OS += macos
	DIDC += didc-macos
endif
ifeq ($(detected_OS),Linux)		  # Ubuntu
	OS += linux
	DIDC += didc-linux64 
endif

ifeq ($(detected_OS),Windows_NT)  # Windows (icpp supports it but you cannot run this Makefile)
	OS += windows_cannot_run_make
endif
ifeq ($(detected_OS),Unknown)     # Unknown
	OS += unknown
endif

###########################################################################
# latest release of didc
VERSION_DIDC := $(shell curl --silent "https://api.github.com/repos/dfinity/candid/releases/latest" | grep -e '"tag_name"' | cut -c 16-25)
# version to install for clang
VERSION_CLANG := $(shell cat version_clang.txt)

###########################################################################
# Use some clang tools that come with wasi-sdk
#
# WASI_SDK_COMPILER_VERSION is read directly from the small version_wasi_sdk
# module (no transitive imports). WASI_SDK_COMPILER_ROOT is then constructed
# the same way config_default.py does it (~/.icpp/wasi-sdk/<version>).
#
# We deliberately do NOT import config_default for this — that module imports
# icpp_candid, which is not yet installed at the time `make install-python`
# parses the Makefile, and on CI runners where pip and python resolve to
# different interpreters (a system Python 3.14 at /Library/Frameworks vs the
# conda env's Python 3.13) it can stay broken even after install. Computing
# the path in pure Make keeps `make` self-bootstrapping.
WASI_SDK_COMPILER_VERSION := $(shell python -c "import sys; sys.path.append('src'); from src.icpp.version_wasi_sdk import __version__; print(__version__)")
WASI_SDK_COMPILER_ROOT := $(HOME)/.icpp/wasi-sdk/$(WASI_SDK_COMPILER_VERSION)
CLANG_FORMAT = $(WASI_SDK_COMPILER_ROOT)/bin/clang-format
CLANG_TIDY = $(WASI_SDK_COMPILER_ROOT)/bin/clang-tidy

.PHONY: summary
summary:
	@echo "-------------------------------------------------------------"
	@echo OS=$(OS)
	@echo VERSION_DIDC=$(VERSION_DIDC)
	@echo VERSION_CLANG=$(VERSION_CLANG)
	@echo WASI_SDK_COMPILER_VERSION=$(WASI_SDK_COMPILER_VERSION)
	@echo WASI_SDK_COMPILER_ROOT=$(WASI_SDK_COMPILER_ROOT)
	@echo CLANG_FORMAT=$(CLANG_FORMAT)
	@echo CLANG_TIDY=$(CLANG_TIDY)
	@echo "-------------------------------------------------------------"

###########################################################################
# CI/CD - Phony Makefile targets
#
.PHONY: all-tests
all-tests: all-static all-canister-native all-canister-deploy-local-pytest 
	
.PHONY: all-canister-deploy-local-pytest
# JOBS = how many canisters to build & test concurrently. Each canister has its
# own local network on an ephemeral port, so they do not collide. The default is
# cpu_count/4 (CI runners have 3-4 vCPU -> 1 job, i.e. serial), because
# icpp build-wasm is itself multi-threaded. Use JOBS=1 to force serial.
JOBS ?=
all-canister-deploy-local-pytest: icpp-pro-test-identity
	@python -m scripts.all_canister_deploy_local_pytest $(if $(JOBS),--jobs $(JOBS),)

# The identity the canisters are deployed with and the tests run as. It is
# named explicitly and passed to every icp command as `--identity`; the
# machine-wide active identity (`icp identity default`) is never read and never
# changed, so running the tests cannot disturb the identity you use for
# mainnet work - and nothing another process does to it can disturb a test run.
ICPP_PRO_TEST_IDENTITY ?= icpp-pro-testing
export ICPP_PRO_TEST_IDENTITY

.PHONY: icpp-pro-test-identity
icpp-pro-test-identity:
	@icp identity list | grep -qw $(ICPP_PRO_TEST_IDENTITY) || \
	  icp identity new $(ICPP_PRO_TEST_IDENTITY) --storage plaintext

.PHONY: all-canister-native
all-canister-native:
	@python -m scripts.all_canister_native

.PHONY: all-static
all-static: \
	cpp-format cpp-lint \
	python-format python-lint python-type
	
CPP_AND_H_FILES = $(shell ls \
icpp-candid/src/icpp_candid/*/*.cpp icpp-candid/src/icpp_candid/*/*.h \
src/icpp/ic/*/*.cpp src/icpp/ic/*/*.h \
src/icpp/canisters/*/src/*.cpp src/icpp/canisters/*/src/*.h \
src/icpp/canisters/*/native/*.cpp src/icpp/canisters/*/native/*.h \
test/canisters/*/src/*.cpp test/canisters/*/src/*.h \
test/canisters/*/native/*.cpp test/canisters/*/native/*.h)

.PHONY: cpp-format
cpp-format:
	@echo "---"
	@echo "cpp-format"
	$(CLANG_FORMAT) --style=file --verbose -i $(CPP_AND_H_FILES)

.PHONY: cpp-lint
cpp-lint:
	@echo "---"
	@echo "cpp-lint"
	@echo "TO IMPLEMENT with clang-tidy"

.PHONY: icp-canisters-of-project-local
icp-canisters-of-project-local:
	@$(eval CANISTER_CANDID_UI_LOCAL := $(shell icp network status --environment local --json | python -c "import json,sys; print(json.load(sys.stdin)['candid_ui_principal'])"))
	@$(eval CANISTER := $(shell icp canister status $(CANISTER_NAME) --environment local --json | python -c "import json,sys; print(json.load(sys.stdin)['id'])"))
	@$(eval GATEWAY_URL := $(shell icp network status --environment local --json | python -c "import json,sys; print(json.load(sys.stdin)['gateway_url'].rstrip('/'))"))

	@echo '-------------------------------------------------'
	@echo "ENVIRONMENT        : local"
	@echo "Candid UI canister : $(CANISTER_CANDID_UI_LOCAL)"
	@echo "canister           : $(CANISTER)"
	@echo '-------------------------------------------------'
	@echo 'View in browser at:'
	@echo  "Candid UI of canister : $(GATEWAY_URL)?canisterId=$(CANISTER_CANDID_UI_LOCAL)&id=$(CANISTER)"

.PHONY: icp-canister-call
icp-canister-call:
	@icp canister call $(CANISTER_NAME) $(CANISTER_METHOD) $(CANISTER_ARGUMENT) --environment $(NETWORK) --output $(CANISTER_OUTPUT) --args-format $(CANISTER_INPUT)

# icp-cli networks are project-local, so run these from a canister directory
.PHONY: icp-start-local
icp-start-local:
	-@icp network stop
	@rm -rf .icp/cache
	@icp network start

.PHONY: icp-stop-local
icp-stop-local:
	@icp network stop

.PHONY: clean-icp
clean-icp:
	rm -rf $(shell find . -name '.icp' -type d)

.PHONY: clean-build
clean-build:
	rm -rf build build-native build-native-unit
	rm -rf $(shell find ./src -name 'build' -type d)
	rm -rf $(shell find ./tests -name 'build' -type d)
	
.PHONY: python-clean
python-clean:
	find . -name '*.pyc' -exec rm -f {} +
	find . -name '*.pyo' -exec rm -f {} +
	find . -name '*~' -exec rm -f  {} +

PYTHON_DIRS ?= test src/icpp

.PHONY: python-format
python-format:
	@echo "---"
	@echo "python-format"
	python -m black $(PYTHON_DIRS)

.PHONY: python-lint
python-lint:
	@echo "---"
	@echo "python-lint"
	python -m pylint --jobs=0 --rcfile=.pylintrc $(PYTHON_DIRS)

.PHONY: python-type
python-type:
	@echo "---"
	@echo "python-type"
	python -m mypy --config-file .mypy.ini --show-column-numbers --strict $(PYTHON_DIRS)


###########################################################################
# Toolchain installation for .github/workflows

# Note for clang++
# This command does not contain latest LLVM version that ships with wasi-sdk
# sudo apt-get update && sudo apt-get install clang-$(VERSION_CLANG)

.PHONY: install-clang-ubuntu
install-clang-ubuntu:
	@echo "Installing clang-$(VERSION_CLANG) compiler"
	# sudo apt-get remove python3-lldb-14
	wget https://apt.llvm.org/llvm.sh
	chmod +x llvm.sh
	echo | sudo ./llvm.sh $(VERSION_CLANG)
	rm llvm.sh

	@echo "Creating soft links for compiler executables"
	sudo ln --force -s /usr/bin/clang-$(VERSION_CLANG) /usr/bin/clang
	sudo ln --force -s /usr/bin/clang++-$(VERSION_CLANG) /usr/bin/clang++


# icp-cli is the successor of the deprecated dfx.
# It is distributed as an npm package & needs >= 1.2.0
.PHONY: install-icp
install-icp:
	npm install -g @icp-sdk/icp-cli

.PHONY: install-didc
install-didc:
	@echo "Installing didc $(VERSION_DIDC) ..."
	sudo rm -rf /usr/local/bin/didc
	wget https://github.com/dfinity/candid/releases/download/${VERSION_DIDC}/$(DIDC)
	sudo mv $(DIDC) /usr/local/bin/didc
	chmod +x /usr/local/bin/didc
	@echo " "
	@echo "Installed successfully in:"
	@echo /usr/local/bin/didc

.PHONY: install-jp-ubuntu
install-jp-ubuntu:
	sudo apt-get update && sudo apt-get install jp

.PHONY: install-jp-mac
install-jp-mac:
	brew install jp

.PHONY: install-homebrew-mac
install-homebrew-mac:
	/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

.PHONY: install-python
install-python:
	# Use `python -m pip` rather than `pip`. On GitHub macOS runners the
	# bare `pip` can resolve to a system Python framework (e.g.
	# /Library/Frameworks/Python.framework/.../pip for Python 3.14) while
	# `python` is the conda env's interpreter — installs land in the wrong
	# site-packages and `import icpp_candid` later fails. `python -m pip`
	# is always the active interpreter's pip.
	#
	# `ensurepip` first: conda envs created via setup-miniconda with
	# channel-priority=strict do NOT include pip by default, so the very
	# first `python -m pip` would fail with "No module named pip".
	# ensurepip is stdlib (no network) and bootstraps the bundled pip.
	python -m ensurepip --upgrade
	python -m pip install --upgrade pip
	cd icpp-candid && rm -rf src/*.egg-info && python -m pip install -e ".[dev]"
	rm -rf src/*.egg-info
	python -m pip install -e ".[dev]"

.PHONY: install-python-w-demos
install-python-w-demos:
	# See the install-python comment above for `ensurepip` and `python -m pip`.
	python -m ensurepip --upgrade
	python -m pip install --upgrade pip
	cd icpp-candid && rm -rf src/*.egg-info && python -m pip install -e ".[dev]"
	rm -rf src/*.egg-info
	python -m pip install -e ".[dev]"
	cd ../icpp-demos && python -m pip install -r requirements.txt


.PHONY: install-python-w-icpp-llm
install-python-w-icpp-llm:
	python -m ensurepip --upgrade
	python -m pip install --upgrade pip
	cd icpp-candid && rm -rf src/*.egg-info && python -m pip install -e ".[dev]"
	rm -rf src/*.egg-info
	python -m pip install -e ".[dev]"
	cd ../icpp_llm && python -m pip install -r requirements.txt

.PHONY: install-python-w-llama_cpp_canister
install-python-w-llama_cpp_canister:
	python -m ensurepip --upgrade
	python -m pip install --upgrade pip
	cd icpp-candid && rm -rf src/*.egg-info && python -m pip install -e ".[dev]"
	rm -rf src/*.egg-info
	python -m pip install -e ".[dev]"
	cd ../llama_cpp_canister && python -m pip install -r requirements.txt

# .PHONY:install-rust
# install-rust:
# 	@echo "Installing rust"
# 	curl https://sh.rustup.rs -sSf | sh -s -- -y
# 	@echo "Installing ic-cdk-optimizer"
# 	cargo install ic-cdk-optimizer

.PHONY: install-wabt
install-wabt:
	sudo apt-get update && sudo apt-get install wabt

###########################################################################
# Building and publishing the pypi package
.PHONY: pypi-build
pypi-build:
	rm -rf dist
	python -m build

.PHONY: testpypi-upload
testpypi-upload:
	twine upload --config-file .pypirc -r testpypi dist/*

.PHONY: testpypi-install
testpypi-install:
	pip install -i https://test.pypi.org/simple/ --extra-index-url https://pypi.org/simple/ icpp

.PHONY: pypi-upload
pypi-upload:
	twine upload --config-file .pypirc dist/*

.PHONY: pypi-install
pypi-install:
	pip install icpp
