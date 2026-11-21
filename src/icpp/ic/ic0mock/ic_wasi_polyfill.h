#include <cstddef>
#include <cstdint>
void raw_init(char *p, std::size_t len);
void raw_init_seed(const unsigned char *seed, std::size_t len);
int32_t raw_set_chunk_size(uint32_t size);
