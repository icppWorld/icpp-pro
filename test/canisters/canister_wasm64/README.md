# Heap memory tests for wasm64-wasi

This canister was created to test wasm64-wasi, which brings >4Gb working memory to canisters !

There are 3 query functions:

1. greet, which does not allocate any static/global memory on the heap
2. greet32, which allocates <4Gb of static/global memory on the heap
3. greet64, which allocates >4Gb of static/global memory on the heap. This traps in wasm32-wasi

## Install

See [installation docs](https://docs.icpp.world/installation.html) for additional details

```bash
pip install icpp-pro

# Verify your version has the build-wasm --target option
$ icpp build-wasm --help

 Usage: icpp build-wasm [OPTIONS]

 Builds the wasm for a canister, using the wasi-sdk compiler.
 Reads icpp.toml in the current folder; Compiles & builds a wasm file.

╭─ Options ───────────────────────────────────────────────────────────────────────────────────────────────────────────────────╮
│ --target                   TEXT  Generate 32-bit or 64-bit wasm [wasm32-wasi/wasm64-wasi]. [default: wasm32-wasi]           │
│ --to-compile               TEXT  Files to compile [all/mine]. [default: all]                                                │
│ --generate-bindings        TEXT  Generate Javascript bindings [yes/no]. [default: yes]                                      │
│ --help                           Show this message and exit.                                                                │
╰─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────╯
```

## Build the wasm

```bash
# 32bit (default)
icpp build-wasm --target wasm32-wasi

# 64bit
icpp build-wasm --target wasm64-wasi
```

## deploy

```bash
dfx deploy
```

## test

```bash
dfx canister call greet query_greet
dfx canister call greet query_greet32
dfx canister call greet query_greet64
dfx canister call greet update_greet
dfx canister call greet update_greet32
dfx canister call greet update_greet64

# Or, just run
pytest
```
