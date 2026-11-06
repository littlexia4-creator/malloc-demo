#include "common.hpp"
#include <limits>

void test_size_overflow(int mmap_limit) {
    assert(m_mallopt(M_MMAP_MAX, mmap_limit));
    for (size_t offset = 0; offset < 64; ++offset)
        assert(m_malloc(std::numeric_limits<size_t>::max() - offset) == nullptr);
    // Rejected requests must leave the allocator usable.
    void* p = m_malloc(64);
    assert(p);
    std::memset(p, 0x5a, 64);
    m_free(p);
}

int main() {
    const int mmap_limits[] = {0, 65536};
    for (int limit : mmap_limits)
        test_size_overflow(limit);
}
