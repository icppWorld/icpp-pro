"""
Defines the version of ic-wasi-polyfill.

(-) Release scripts might modify this automatically...
(-) do not add anything but the version number of ic-wasi-polyfill here!

"""

# https://github.com/wasm-forge/ic-wasi-polyfill - v0.13.2 (stable-fs 0.13.1)
# with raw_set_chunk_size, at the commit before the tag: the tag's Cargo.lock
# still names 0.13.1, so `cargo build --locked` fails there. The tag adds only
# that version bump and a fix of the report_wasi_calls debug feature
__version__ = "3ec6bf4512e924a4b39894ed5fe8fd0ae16ea3bd"
# Before 7.1.0rc6: 9f379a4c782450ce4950da2279f4530934ca695c (v0.13.1)
# Before 7.1.0rc1: b788d8d6fe80deb0d8e27198ebeee5045204302f (stable-fs 0.13.0)
# Before 7.0.0rc3: b3ef005140e7eebf7d0b5471ccc3a6d4cbec4ee5 (stable-fs 0.5.1)

# https://github.com/wasm-forge/ic-wasi-polyfill/tags
# __version__ = "v0.5.0"
# __version__ = "v0.4.3"
