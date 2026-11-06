#include "common.hpp"
#include <cstdio>

struct Allocation {
    unsigned char* data = nullptr;
    size_t size = 0;
    unsigned char pattern = 0;
};

static void verify(const Allocation& a) {
    if (!a.data) return;
    for (size_t i = 0; i < a.size; ++i) {
        if (a.data[i] != a.pattern) {
            std::fprintf(stderr, "corruption at byte %zu: expected %u, got %u\n",
                         i, unsigned(a.pattern), unsigned(a.data[i]));
            std::abort();
        }
    }
}

int main() {
    Allocation slots[100];
    unsigned state = 12345;
    auto random = [&]() { state = state * 1664525u + 1013904223u; return state; };
    for (size_t iteration = 0; iteration < 10000; ++iteration) {
        auto& a = slots[random() % 100];
        verify(a);
        if (a.data) {
            m_free(a.data);
            a = {};
        } else {
            a.size = 1 + random() % 8192;
            a.pattern = static_cast<unsigned char>(random() >> 24);
            a.data = static_cast<unsigned char*>(m_malloc(a.size));
            assert(a.data);
            std::memset(a.data, a.pattern, a.size);
        }
        // Check every byte of every live allocation, even after iteration 255.
        for (const auto& live : slots) verify(live);
    }
    for (auto& a : slots) {
        verify(a);
        m_free(a.data);
    }
}
