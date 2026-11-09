#include "common.hpp"
#include <cerrno>
#include <limits>
#include <sys/resource.h>

unsigned char pattern_at(size_t index) {
    return static_cast<unsigned char>(index * 37 + 19);
}

void verify_payload(const unsigned char* p, size_t size) {
    for (size_t i = 0; i < size; ++i) assert(p[i] == pattern_at(i));
}

void fill_payload(unsigned char* p, size_t size) {
    for (size_t i = 0; i < size; ++i) p[i] = pattern_at(i);
}

void test_realloc_null_and_zero() {
    auto* p = static_cast<unsigned char*>(m_realloc(nullptr, 64));
    assert(p);
    assert(m_malloc_usable_size(p) >= 64);
    fill_payload(p, 64);
    verify_payload(p, 64);
    assert(m_realloc(p, 0) == nullptr);

    // A null pointer follows malloc(0); either null or a freeable block is valid.
    m_free(m_realloc(nullptr, 0));
}

void test_realloc_preserves_data(int mmap_limit) {
    assert(m_mallopt(M_MMAP_MAX, mmap_limit));
    assert(m_mallopt(M_MMAP_THRESHOLD, 128 * 1024));
    const size_t sizes[] = {1, 32, 32, 129, 8192, 256 * 1024,
                            512 * 1024, 256, 4096, 64};
    unsigned char* p = nullptr;
    size_t previous_size = 0;
    for (size_t size : sizes) {
        auto* resized = static_cast<unsigned char*>(m_realloc(p, size));
        assert(resized);
        assert(m_malloc_usable_size(resized) >= size);
        verify_payload(resized, std::min(previous_size, size));
        fill_payload(resized, size);
        verify_payload(resized, size);
        p = resized;
        previous_size = size;
    }
    m_free(p);
}

void test_realloc_mapped_statistics() {
    assert(m_mallopt(M_MMAP_MAX, 65536));
    assert(m_mallopt(M_MMAP_THRESHOLD, 128 * 1024));
    const auto before = m_mallinfo2();
    size_t size = 2 * 1024 * 1024;
    auto* p = static_cast<unsigned char*>(m_malloc(size));
    assert(p);
    fill_payload(p, size);
    auto previous = m_mallinfo2();
    assert(previous.hblks == before.hblks + 1);

    // Resizing an existing mapping does not require another mmap slot.
    assert(m_mallopt(M_MMAP_MAX, 0));
    for (size_t next_size : {size_t(4 * 1024 * 1024), size_t(256 * 1024)}) {
        auto* resized = static_cast<unsigned char*>(m_realloc(p, next_size));
        assert(resized);
        assert(m_malloc_usable_size(resized) >= next_size);
        verify_payload(resized, std::min(size, next_size));
        fill_payload(resized, next_size);
        const auto current = m_mallinfo2();
        assert(current.hblks == previous.hblks);
        if (next_size > size) assert(current.hblkhd > previous.hblkhd);
        else assert(current.hblkhd < previous.hblkhd);
        p = resized;
        size = next_size;
        previous = current;
    }

    // A small shrink within the same page keeps the mapping and its capacity.
    assert(m_realloc(p, size - 1) == p);
    verify_payload(p, size - 1);
    assert(m_mallinfo2().hblkhd == previous.hblkhd);
    m_free(p);
    const auto after = m_mallinfo2();
    assert(after.hblks == before.hblks);
    assert(after.hblkhd == before.hblkhd);
    assert(m_mallopt(M_MMAP_MAX, 65536));
}

void test_realloc_overflow_preserves_original(size_t size) {
    auto* p = static_cast<unsigned char*>(m_malloc(size));
    assert(p);
    fill_payload(p, size);
    volatile size_t impossible_size = std::numeric_limits<size_t>::max();
    errno = 0;
    assert(m_realloc(p, impossible_size) == nullptr);
    assert(errno == ENOMEM);
    verify_payload(p, size);
    assert(m_malloc_usable_size(p) >= size);
    m_free(p);
}

void test_realloc_oom_preserves_original() {
    assert(m_mallopt(M_MMAP_MAX, 0));
    auto* p = static_cast<unsigned char*>(m_malloc(64));
    assert(p);
    fill_payload(p, 64);

    assert(m_mallopt(M_MMAP_MAX, 65536));
    const size_t mapped_size = 2 * 1024 * 1024;
    auto* mapped = static_cast<unsigned char*>(m_malloc(mapped_size));
    assert(mapped);
    fill_payload(mapped, mapped_size);
    const auto before = m_mallinfo2();
    assert(m_mallopt(M_MMAP_MAX, 0));

    // Run last: these process-local limits prevent any further heap growth.
    const rlimit limit{0, 0};
    assert(setrlimit(RLIMIT_AS, &limit) == 0);
    errno = 0;
    assert(m_realloc(p, 64 * 1024 * 1024) == nullptr);
    assert(errno == ENOMEM);
    verify_payload(p, 64);
    assert(m_malloc_usable_size(p) >= 64);

    // Both mremap and the allocation/copy/free fallback must fail safely.
    errno = 0;
    assert(m_realloc(mapped, 64 * 1024 * 1024) == nullptr);
    assert(errno == ENOMEM);
    verify_payload(mapped, mapped_size);
    assert(m_malloc_usable_size(mapped) >= mapped_size);
    const auto after = m_mallinfo2();
    assert(after.hblks == before.hblks);
    assert(after.hblkhd == before.hblkhd);

    errno = 0;
    assert(m_realloc(nullptr, 64 * 1024 * 1024) == nullptr);
    assert(errno == ENOMEM);
    m_free(mapped);
    m_free(p);
}

int main() {
    test_realloc_null_and_zero();
    test_realloc_preserves_data(0);
    test_realloc_preserves_data(65536);
    test_realloc_mapped_statistics();
    test_realloc_overflow_preserves_original(64);
    test_realloc_overflow_preserves_original(2 * 1024 * 1024);
    test_realloc_oom_preserves_original();
}
