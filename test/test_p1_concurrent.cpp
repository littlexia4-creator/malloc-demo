#include "common.hpp"
#include <atomic>
#include <thread>

template <typename Work, typename Observe>
void run_concurrently(Work work, Observe observe) {
    assert(m_mallopt(M_MMAP_MAX, 2));
    assert(m_mallopt(M_MMAP_THRESHOLD, 128 * 1024));
    std::atomic<int> ready{0};
    std::atomic<int> done{0};
    std::atomic<bool> start{false};
    std::thread workers[4];
    for (int id = 0; id < 4; ++id) {
        workers[id] = std::thread([&, id] {
            ++ready;
            while (!start.load()) std::this_thread::yield();
            work(id);
            ++done;
        });
    }
    while (ready.load() != 4) std::this_thread::yield();
    start = true;
    while (done.load() != 4) {
        observe();
        std::this_thread::yield();
    }
    for (auto& worker : workers) worker.join();
}

void allocate_and_check(int id) {
    for (int i = 0; i < 10000; ++i) {
        // Exercise mmap, heap growth, and free-list reuse.
        const size_t size = i % 256 == 0 ? 128 * 1024 : (i % 32 == 0 ? 8192 : 64);
        auto* p = static_cast<unsigned char*>(m_malloc(size));
        assert(p);
        assert(m_malloc_usable_size(p) >= size);
        std::memset(p, id + 1, size);
        for (size_t j = 0; j < size; ++j) assert(p[j] == id + 1);
        m_free(p);
    }
}

unsigned char* allocate_and_fill(size_t size, int pattern) {
    auto* p = static_cast<unsigned char*>(m_malloc(size));
    assert(p);
    std::memset(p, pattern, size);
    return p;
}

void test_concurrent_initialization() {
    unsigned char* blocks[4] = {};
    run_concurrently([&](int id) {
        blocks[id] = allocate_and_fill(64, id + 1);
    }, [] {});
    // Keep the first allocations alive until every thread has finished.
    for (int id = 0; id < 4; ++id) {
        for (int other = id + 1; other < 4; ++other)
            assert(blocks[id] != blocks[other]);
        for (size_t j = 0; j < 64; ++j) assert(blocks[id][j] == id + 1);
    }
    for (auto* p : blocks) m_free(p);
}

void test_concurrent_allocation_free() {
    run_concurrently(allocate_and_check, [] {});
}

void test_concurrent_trim_and_statistics() {
    run_concurrently(allocate_and_check, [] {
        assert(m_mallopt(M_MMAP_THRESHOLD, 128 * 1024));
        m_malloc_trim(0);
        const auto info = m_mallinfo2();
        assert(info.keepcost <= info.fordblks);
        assert(info.fordblks <= info.arena);
        assert(info.hblks <= 2);
    });
}

int main() {
    // Run initialization first, before any test calls malloc.
    test_concurrent_initialization();
    test_concurrent_allocation_free();
    test_concurrent_trim_and_statistics();
}
