#include "common.hpp"

void test_malloc_free() {
    // 测试基本的malloc功能
    void* ptr1 = m_malloc(100);
    assert(ptr1 != nullptr);

    // 测试不同大小的内存分配
    void* ptr2 = m_malloc(1024);
    assert(ptr2 != nullptr);
    assert(ptr2 != ptr1);  // 应该分配不同的地址

    // 测试小内存分配
    void* ptr3 = m_malloc(1);
    assert(ptr3 != nullptr);
    assert(ptr3 != ptr1 && ptr3 != ptr2);

    // 测试内存写入和读取
    char* char_ptr = static_cast<char*>(ptr1);
    const char test_data[] = "Hello, malloc!";
    void* dest = std::memcpy(char_ptr, test_data, sizeof(test_data));
    assert(dest != nullptr);
    assert(dest == ptr1);
    assert(std::memcmp(char_ptr, test_data, sizeof(test_data)) == 0);

    // 测试零大小分配（行为可能因实现而异）
    void* ptr4 = m_malloc(0);
    assert(ptr4 != nullptr);  // 根据实现，可能返回nullptr或有效指针
    // 不能对ptr4进行写入

    // 测试free功能
    try
    {
        // 测试释放nullptr（应该安全）
        m_free(nullptr);
    }
    catch (...) {
        assert(false && "释放nullptr时不应该抛出异常");
    }
    try
    {
        m_free(ptr1);
    }
    catch (...) {
        assert(false);
    }
    try
    {
        m_free(ptr2);
    }
    catch (...) {
        assert(false);
    }
    try
    {
        m_free(ptr3);
    }
    catch (...) {
        assert(false);
    }
    try
    {
        m_free(ptr4);
    }
    catch (...) {
        assert(false);
    }

    // 测试多次分配和释放
    constexpr size_t test_count = 10;
    void* ptrs[test_count];

    for (size_t i = 0; i < test_count; ++i) {
        ptrs[i] = m_malloc((i + 1) * 100);
        assert(ptrs[i] != nullptr);
        std::memcpy(static_cast<char*>(ptrs[i]), "Test data", 9);
        assert(ptrs[i] != nullptr);
    }

    for (size_t i = 0; i < test_count; ++i) {
        m_free(ptrs[i]);
    }

    // 测试分配超大内存
    void* large_ptr = m_malloc(1024 * 1024 * 1024); // 分配1GB
    if (large_ptr == nullptr) {
        std::cerr << "分配1GB内存失败, 可能是内存不足" << std::endl;
    } else {
        // 对超大内存进行读写
        char* large_char_ptr = static_cast<char*>(large_ptr) + 1024 * 1024 * 512;
        // 生成长文本 large_test_data
        const char large_test_data[] = "Large memory allocation test: "
                                       "This is a test to check if the malloc and free functions can handle large memory allocations without issues. "
                                       "We will fill this memory with some data to ensure it works correctly.";
        void* large_dest = std::memcpy(large_char_ptr, large_test_data, sizeof(large_test_data));
        assert(large_dest != nullptr);
        assert(large_dest == large_char_ptr);
        assert(std::memcmp(large_char_ptr, large_test_data, sizeof(large_test_data)) == 0);
    }

    try
    {
        m_free(large_ptr);
    }
    catch (...) {
        assert(false && "释放nullptr时不应该抛出异常");
    }

    std::cout << "自定义malloc和free函数测试全部通过!" << std::endl;
}

int main() { test_malloc_free(); }
