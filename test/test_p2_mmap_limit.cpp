#include "common.hpp"

int main() {
    assert(m_mallopt(M_MMAP_MAX, 2));
    void* blocks[3];
    for (int i = 0; i < 3; ++i) {
        blocks[i] = m_malloc(128 * 1024);
        assert(blocks[i]);
        assert(m_mallinfo2().hblks == size_t(std::min(i + 1, 2)));
    }
    for (void* p : blocks) m_free(p);
    assert(m_mallinfo2().hblks == 0);
}
