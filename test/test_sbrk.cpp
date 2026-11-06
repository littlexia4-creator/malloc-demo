#include "common.hpp"
#include "../sbrk.hpp"

void test_sbrk() {
    // 测试获取当前断点
    void* initial_brk = m_sbrk(0);
    assert(initial_brk != reinterpret_cast<void*>(-1));
    assert(initial_brk != nullptr);

    // 测试增加堆空间
    constexpr intptr_t increment = 2048;  // 增加2KB
    void* old_brk = m_sbrk(increment);
    assert(old_brk != reinterpret_cast<void*>(-1));
    assert(old_brk == initial_brk);

    // 验证断点确实增加了
    void* current_brk = m_sbrk(0);
    assert(current_brk != reinterpret_cast<void*>(-1));
    assert(reinterpret_cast<uintptr_t>(current_brk) ==
           reinterpret_cast<uintptr_t>(initial_brk) + increment);

    // 测试减少堆空间
    constexpr intptr_t decrement = -increment; // 减少2KB
    void* old_brk2 = m_sbrk(decrement);
    assert(old_brk2 != reinterpret_cast<void*>(-1));
    assert(old_brk2 == current_brk);

    // 验证断点恢复到初始状态
    void* final_brk = m_sbrk(0);
    assert(final_brk != reinterpret_cast<void*>(-1));
    assert(final_brk == initial_brk);

    std::cout << "自定义sbrk函数测试通过!" << std::endl;
}

int main() { test_sbrk(); }
