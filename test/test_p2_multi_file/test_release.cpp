#include "../common.hpp"
#include <cassert>
void* allocate();
size_t allocated_bytes();
int main() {
    void* p = allocate();
    assert(p);
    assert(m_malloc_usable_size(p) >= 64);
    assert(allocated_bytes() == m_mallinfo2().uordblks);
    for (size_t i = 0; i < 64; ++i)
        assert(static_cast<unsigned char*>(p)[i] == 0x5a);
    m_free(p);
    assert(allocated_bytes() == m_mallinfo2().uordblks);
}
