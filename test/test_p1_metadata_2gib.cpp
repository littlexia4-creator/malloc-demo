#include "common.hpp"

int main() {
    static_assert(sizeof(size_t) >= 8, "requires 64-bit Linux");
    const size_t requested = size_t(1) << 31;
    const auto mappings_before = m_mallinfo2().hblks;
    void* p = m_malloc(requested);
    // Rejection is safe. Success must provide the complete payload.
    if (!p) return 0;
    assert(m_malloc_usable_size(p) >= requested);
    auto* bytes = static_cast<volatile unsigned char*>(p);
    bytes[0] = 0x55;
    bytes[requested - 1] = 0xaa;
    assert(bytes[0] == 0x55 && bytes[requested - 1] == 0xaa);
    m_free(p);
    assert(m_mallinfo2().hblks == mappings_before);
}
