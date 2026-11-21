#include <stdint.h>

// The default when no generated definition is linked: keep the size the file
// system has. icpp build-wasm links a strong definition, which wins
__attribute__((weak)) const uint32_t icpp_stable_fs_chunk_size = 0;
