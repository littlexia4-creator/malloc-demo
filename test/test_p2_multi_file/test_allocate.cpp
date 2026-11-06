#include "../common.hpp"
void* allocate() {
    void* p = m_malloc(64);
    assert(p);
    std::memset(p, 0x5a, 64);
    return p;
}

size_t allocated_bytes() { return m_mallinfo2().uordblks; }
