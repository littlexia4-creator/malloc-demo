#include "common.hpp"

int main() {
    assert(m_mallopt(M_MMAP_MAX, 0));
    assert(m_mallopt(M_TRIM_THRESHOLD, -1));
    void* p = m_malloc(200000);
    assert(p);
    m_free(p);
    const auto before = m_mallinfo2().keepcost;
    const int result = m_malloc_trim(0);
    assert(m_mallinfo2().keepcost < before);
    assert(result == 1);
    assert(m_malloc_trim(0) == 0);
}
