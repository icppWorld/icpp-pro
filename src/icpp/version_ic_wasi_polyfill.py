"""
Defines the version of ic-wasi-polyfill.

(-) Release scripts might modify this automatically...
(-) do not add anything but the version number of ic-wasi-polyfill here!

"""

# https://github.com/wasm-forge/ic-wasi-polyfill - tag v0.13.2 (stable-fs 0.13.1),
# with raw_set_chunk_size. Upstream has no Cargo.lock: icpp's own is in
# cargo_locks/, and names the version of the tag
# git rev-parse v0.13.2
__version__ = "0af85e782da593f9f239b2f925aa19934516ce2d"
# Before 7.1.0rc6: 9f379a4c782450ce4950da2279f4530934ca695c (v0.13.1)
# Before 7.1.0rc1: b788d8d6fe80deb0d8e27198ebeee5045204302f (stable-fs 0.13.0)
# Before 7.0.0rc3: b3ef005140e7eebf7d0b5471ccc3a6d4cbec4ee5 (stable-fs 0.5.1)

# https://github.com/wasm-forge/ic-wasi-polyfill/tags
# __version__ = "v0.5.0"
# __version__ = "v0.4.3"
