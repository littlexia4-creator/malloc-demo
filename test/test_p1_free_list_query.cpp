#include "common.hpp"
#include <atomic>
#include <thread>

void test_concurrent_free_list_query() {
    assert(m_mallopt(M_MMAP_MAX, 0));
    assert(m_mallopt(M_TRIM_THRESHOLD, 0));
    std::atomic<bool> start{false};
    std::atomic<bool> done{false};
    std::thread worker([&] {
        while (!start.load()) std::this_thread::yield();
        for (int i = 0; i < 10000; ++i) {
            void* a = m_malloc(200000);
            void* b = m_malloc(200000);
            void* c = m_malloc(200000);
            void* guard = m_malloc(64);
            assert(a && b && c && guard);
            m_free(a);
            m_free(c);
            std::this_thread::yield();
            m_free(b);
            m_free(guard);
        }
        done = true;
    });

    start = true;
    do {
        // Query directly while the worker changes bins and returns pages.
        const auto info = xiaoyu::get_free_blocks_info();
        assert(info.second >= info.first * EMPTY_BLOCK_SIZE);
    } while (!done.load());
    worker.join();
    assert(xiaoyu::get_free_blocks_info().first == 0);
}

int main() { test_concurrent_free_list_query(); }
