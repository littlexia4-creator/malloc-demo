#include "common.hpp"

void test_mallopt() {
    assert(m_mallopt(M_TOP_PAD, 128 * 1024)); // 设置top_pad为128KB
    assert(m_mallopt(M_TOP_PAD, 0)); // 设置top_pad为0, 无需保持top Chunk的大小
    M_TOP_PAD 不能设置为 -1
    assert(m_mallopt(M_MMAP_THRESHOLD, 128 * 1024));
    assert(m_mallopt(M_MMAP_THRESHOLD, 0)); // 优先使用mmap
    assert(m_mallopt(M_MMAP_THRESHOLD, -1)); // 禁用 mmap 分配
    assert(m_mallopt(M_TRIM_THRESHOLD, 128 * 1024));
    assert(m_mallopt(M_TRIM_THRESHOLD, 0));  // 立马归还堆区内存
    assert(m_mallopt(M_TRIM_THRESHOLD, -1)); // 禁用 trim 功能
    assert(m_mallopt(M_MMAP_MAX, 0)); // 禁用 mmap 分配
}

int main() { test_mallopt(); }
