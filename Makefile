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
ICPP_COMPILER_VERSION := $(shell python -c "import sys; sys.path.append('src'); from src.icpp.version_wasi_sdk import __version__; print(__version__)")
ICPP_COMPILER_ROOT := $(HOME)/.icpp/$(ICPP_COMPILER_VERSION)
CLANG_FORMAT = $(ICPP_COMPILER_ROOT)/bin/clang-format
CLANG_TIDY = $(ICPP_COMPILER_ROOT)/bin/clang-tidy

.PHONY: summary
summary:
	@echo "-------------------------------------------------------------"
	@echo OS=$(OS)
	@echo VERSION_DIDC=$(VERSION_DIDC)
	@echo VERSION_CLANG=$(VERSION_CLANG)
	@echo ICPP_COMPILER_VERSION=$(ICPP_COMPILER_VERSION)
	@echo ICPP_COMPILER_ROOT=$(ICPP_COMPILER_ROOT)
	@echo CLANG_FORMAT=$(CLANG_FORMAT)
	@echo CLANG_TIDY=$(CLANG_TIDY)
	@echo "-------------------------------------------------------------"

###########################################################################
# CI/CD - Phony Makefile targets
#
.PHONY: all-tests
all-tests: all-static all-canister-deploy-local-pytest 
	
.PHONY: all-canister-deploy-local-pytest
all-canister-deploy-local-pytest:
	dfx identity use default
	@python -m scripts.all_canister_deploy_local_pytest

.PHONY: all-canister-native
all-canister-native:
	@python -m scripts.all_canister_native

.PHONY: all-static
all-static: \
	cpp-format cpp-lint \
	python-format python-lint python-type
	
CPP_AND_H_FILES = $(shell ls \
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

.PHONY: dfx-canisters-of-project-local
dfx-canisters-of-project-local:
	@$(eval CANISTER_WALLET := $(shell dfx identity get-wallet))
	@$(eval CANISTER_CANDID_UI_LOCAL := $(shell dfx canister id __Candid_UI))
	@$(eval CANISTER := $(shell dfx canister id $(CANISTER_NAME)))

	@echo '-------------------------------------------------'
	@echo "NETWORK            : local"
	@echo "cycles canister    : $(CANISTER_WALLET)"
	@echo "Candid UI canister : $(CANISTER_CANDID_UI_LOCAL)"
	@echo "canister           : $(CANISTER)"
	@echo '-------------------------------------------------'
	@echo 'View in browser at:'
	@echo  "Candid UI of canister : http://localhost:8000?canisterId=$(CANISTER_CANDID_UI_LOCAL)&id=$(CANISTER)"

.PHONY: dfx-canister-call
dfx-canister-call:
	@dfx canister --network $(NETWORK) call --output $(CANISTER_OUTPUT) --type $(CANISTER_INPUT) $(CANISTER_NAME) $(CANISTER_METHOD) $(CANISTER_ARGUMENT)

.PHONY: dfx-start-local
dfx-start-local:
	@dfx stop
	@dfx start --clean

.PHONY: dfx-stop-local
dfx-stop-local:
	@dfx stop

.PHONY: clean-dfx
clean-dfx:
	rm -rf $(shell find . -name '.dfx' -type d)

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
# sudo apt update && sudo apt install clang-$(VERSION_CLANG)

.PHONY: install-clang-ubuntu
install-clang-ubuntu:
	@echo "Installing clang-$(VERSION_CLANG) compiler"
	wget https://apt.llvm.org/llvm.sh
	chmod +x llvm.sh
	echo | sudo ./llvm.sh $(VERSION_CLANG)
	rm llvm.sh

	@echo "Creating soft links for compiler executables"
	sudo ln --force -s /usr/bin/clang-$(VERSION_CLANG) /usr/bin/clang
	sudo ln --force -s /usr/bin/clang++-$(VERSION_CLANG) /usr/bin/clang++


# This installs ~/bin/dfx
# Make sure to source ~/.profile afterwards -> it adds ~/bin to the path if it exists
.PHONY: install-dfx
install-dfx:
	sh -ci "$$(curl -fsSL https://sdk.dfinity.org/install.sh)"

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

.PHONY: install-jp
install-jp:
	sudo apt-get update && sudo apt-get install jp

.PHONY: install-python
install-python:
	pip install --upgrade pip
	cd icpp-candid && pip install -e ".[dev]"
	pip install -e ".[dev]"

.PHONY:install-rust
install-rust:
	@echo "Installing rust"
	curl https://sh.rustup.rs -sSf | sh -s -- -y
	@echo "Installing ic-cdk-optimizer"
	cargo install ic-cdk-optimizer

.PHONY: install-wabt
install-wabt:
	sudo apt update && sudo apt install wabt

###########################################################################
# Building and publishing the pypi package
.PHONY: pypi-build
pypi-build:
	rm -rf dist
	python -m build

.PHONY: testpypi-upload
testpypi-upload:
	twine upload --config-file .pypirc -r testpypi dist/*.whl

.PHONY: testpypi-install
testpypi-install:
	pip install -i https://test.pypi.org/simple/ --extra-index-url https://pypi.org/simple/ icpp

.PHONY: pypi-upload
pypi-upload:
	twine upload --config-file .pypirc dist/*.whl

.PHONY: pypi-install
pypi-install:
	pip install icpp