#include "common.hpp"

void test_malloc_additional() {
    std::cout << "开始更多malloc断言测试..." << std::endl;

    // 测试1: 内存对齐检查
    std::cout << "测试内存对齐..." << std::endl;
    for (size_t size = 1; size <= 1024; size *= 2) {
        void* ptr = m_malloc(size);
        assert(ptr != nullptr);

        // 检查内存对齐（应该是8字节或16字节对齐）
        uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
        #if defined(__x86_64__) || defined(_M_X64) || defined(__aarch64__)
            assert((addr % 16) == 0 && "内存应该16字节对齐");
        #else
            assert((addr % 8) == 0 && "内存应该8字节对齐");
        #endif

        m_free(ptr);
    }
    std::cout << "内存对齐测试通过!" << std::endl;

    // 测试2: 边界值测试
    std::cout << "测试边界值..." << std::endl;

    // 测试最小分配大小
    void* min_ptr = m_malloc(1);
    assert(min_ptr != nullptr);
    *(char*)min_ptr = 'A';
    assert(*(char*)min_ptr == 'A');
    m_free(min_ptr);

    // 测试各种奇怪的大小
    size_t weird_sizes[] = {3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97};
    void* weird_ptrs[sizeof(weird_sizes) / sizeof(weird_sizes[0])];

    for (size_t i = 0; i < sizeof(weird_sizes) / sizeof(weird_sizes[0]); ++i) {
        weird_ptrs[i] = m_malloc(weird_sizes[i]);
        assert(weird_ptrs[i] != nullptr);

        // 写入测试数据
        memset(weird_ptrs[i], i + 1, weird_sizes[i]);
    }

    // 验证数据完整性
    for (size_t i = 0; i < sizeof(weird_sizes) / sizeof(weird_sizes[0]); ++i) {
        unsigned char* data = static_cast<unsigned char*>(weird_ptrs[i]);
        for (size_t j = 0; j < weird_sizes[i]; ++j) {
            assert(data[j] == (unsigned char)(i + 1));
        }
        m_free(weird_ptrs[i]);
    }
    std::cout << "边界值测试通过!" << std::endl;

    // 测试3: 内存碎片化和合并测试
    std::cout << "测试内存碎片化和合并..." << std::endl;
    constexpr size_t frag_count = 20;
    void* frag_ptrs[frag_count];

    // 分配多个小块
    for (size_t i = 0; i < frag_count; ++i) {
        frag_ptrs[i] = m_malloc(64 + i * 8);
        assert(frag_ptrs[i] != nullptr);
        memset(frag_ptrs[i], i, 64 + i * 8);
    }

    // 释放一半（奇数索引），创建碎片
    for (size_t i = 1; i < frag_count; i += 2) {
        m_free(frag_ptrs[i]);
        frag_ptrs[i] = nullptr;
    }

    // 验证剩余块的数据完整性
    for (size_t i = 0; i < frag_count; i += 2) {
        unsigned char* data = static_cast<unsigned char*>(frag_ptrs[i]);
        for (size_t j = 0; j < 64 + i * 8; ++j) {
            assert(data[j] == (unsigned char)i);
        }
    }

    // 重新分配一些内存，测试空闲块重用
    for (size_t i = 1; i < frag_count; i += 2) {
        frag_ptrs[i] = m_malloc(32);
        assert(frag_ptrs[i] != nullptr);
        memset(frag_ptrs[i], 255, 32);
    }

    // 清理所有内存
    for (size_t i = 0; i < frag_count; ++i) {
        if (frag_ptrs[i] != nullptr) {
            m_free(frag_ptrs[i]);
        }
    }
    std::cout << "内存碎片化和合并测试通过!" << std::endl;

    // Random allocation stress is in test_data_integrity.cpp.
    struct {
        size_t size;
        unsigned char pattern;
        const char* description;
    } pattern_tests[] = {
        {100, 0x00, "零模式"},
        {200, 0xFF, "全1模式"},
        {150, 0xAA, "交替模式1"},
        {300, 0x55, "交替模式2"},
        {250, 0xCC, "自定义模式"}
    };

    constexpr size_t pattern_count = sizeof(pattern_tests) / sizeof(pattern_tests[0]);
    void* pattern_ptrs[pattern_count];

    for (size_t i = 0; i < pattern_count; ++i) {
        pattern_ptrs[i] = m_malloc(pattern_tests[i].size);
        assert(pattern_ptrs[i] != nullptr);

        // 写入特定模式
        memset(pattern_ptrs[i], pattern_tests[i].pattern, pattern_tests[i].size);
        std::cout << "写入" << pattern_tests[i].description << "完成" << std::endl;
    }

    // 验证所有模式
    for (size_t i = 0; i < pattern_count; ++i) {
        unsigned char* data = static_cast<unsigned char*>(pattern_ptrs[i]);
        for (size_t j = 0; j < pattern_tests[i].size; ++j) {
            assert(data[j] == pattern_tests[i].pattern);
        }
        std::cout << pattern_tests[i].description << "验证通过" << std::endl;
        m_free(pattern_ptrs[i]);
    }
    std::cout << "内存模式验证测试通过!" << std::endl;

    // 测试7: 内存越界写入检测（基本测试）
    std::cout << "测试内存越界基本检测..." << std::endl;
    void* bounds_ptr = m_malloc(50);
    assert(bounds_ptr != nullptr);

    char* char_bounds = static_cast<char*>(bounds_ptr);
    // 写入分配的内存范围内
    for (int i = 0; i < 50; ++i) {
        char_bounds[i] = 'B';
    }

    // 验证写入
    for (int i = 0; i < 50; ++i) {
        assert(char_bounds[i] == 'B');
    }

    m_free(bounds_ptr);

    std::cout << "所有额外的malloc断言测试全部通过!" << std::endl;
}

int main() { test_malloc_additional(); }
