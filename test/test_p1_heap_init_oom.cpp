#include "common.hpp"
#include <cerrno>
#include <sys/resource.h>

void test_heap_initialization_failure() {
    assert(m_mallopt(M_MMAP_MAX, 0));
    // we deliberately prevent memory growth
    const rlimit limit{0, 0};
    assert(setrlimit(RLIMIT_DATA, &limit) == 0);
    assert(setrlimit(RLIMIT_AS, &limit) == 0);

    errno = 0;
    void* p = m_malloc(256 * 1024);
    assert(p == nullptr);
    assert(errno == ENOMEM);
}

int main() { test_heap_initialization_failure(); }
