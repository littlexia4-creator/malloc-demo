#include "common.hpp"
#include <atomic>
#include <thread>

int main() {
    m_free(m_malloc(64));
    std::atomic<int> ready{0};
    std::atomic<bool> start{false};
    std::thread workers[4];
    for (int id = 0; id < 4; ++id) {
        workers[id] = std::thread([&, id] {
            ++ready;
            while (!start.load()) std::this_thread::yield();
            for (int i = 0; i < 10000; ++i) {
                auto* p = static_cast<unsigned char*>(m_malloc(64));
                assert(p);
                std::memset(p, id + 1, 64);
                for (int j = 0; j < 64; ++j) assert(p[j] == id + 1);
                m_free(p);
            }
        });
    }
    while (ready.load() != 4) std::this_thread::yield();
    start = true;
    for (auto& worker : workers) worker.join();
}
