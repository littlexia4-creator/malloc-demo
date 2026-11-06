#include "common.hpp"

void test_memory_mmap() {
    std::cout << "开始测试大内存mmap分配..." << std::endl;

    // 测试MMAP_THRESHOLD边界值
    const size_t threshold = 128 * 1024; // 128KB

    // 测试小于阈值的分配（应该使用普通malloc）
    void* small_ptr = m_malloc(threshold - 1);
    assert(small_ptr != nullptr);
    std::cout << "小内存分配（< 128KB）测试通过" << std::endl;

    // 测试等于阈值的分配（应该使用mmap）
    void* threshold_ptr = m_malloc(threshold);
    assert(threshold_ptr != nullptr);
    std::cout << "阈值内存分配（= 128KB）测试通过" << std::endl;

    // 测试大于阈值的分配（应该使用mmap）
    void* large_ptr1 = m_malloc(256 * 1024); // 256KB
    assert(large_ptr1 != nullptr);
    std::cout << "大内存分配（256KB）测试通过" << std::endl;

    // 测试超大内存分配
    void* large_ptr2 = m_malloc(2 * 1024 * 1024); // 2MB
    assert(large_ptr2 != nullptr);
    std::cout << "超大内存分配（2MB）测试通过" << std::endl;

    // 测试对大内存进行读写
    char* char_ptr = static_cast<char*>(large_ptr1);
    const char test_data[] = "大内存mmap分配测试数据";
    memcpy(char_ptr, test_data, sizeof(test_data));
    assert(memcmp(char_ptr, test_data, sizeof(test_data)) == 0);
    std::cout << "大内存读写测试通过" << std::endl;

    // 测试释放大内存
    m_free(small_ptr);
    m_free(threshold_ptr);
    m_free(large_ptr1);
    m_free(large_ptr2);
    std::cout << "大内存释放测试通过" << std::endl;

    // 测试多次大内存分配和释放
    constexpr size_t large_test_count = 5;
    void* large_ptrs[large_test_count];

    for (size_t i = 0; i < large_test_count; ++i) {
        large_ptrs[i] = m_malloc((i + 1) * 512 * 1024); // 512KB, 1MB, 1.5MB, 2MB, 2.5MB
        assert(large_ptrs[i] != nullptr);
        // 写入一些数据
        memset(large_ptrs[i], i + 1, 1024); // 写入前1KB
    }

    for (size_t i = 0; i < large_test_count; ++i) {
        // 验证数据
        char* data = static_cast<char*>(large_ptrs[i]);
        for (size_t j = 0; j < 1024; ++j) {
            assert(data[j] == static_cast<char>(i + 1));
        }
        m_free(large_ptrs[i]);
    }

    std::cout << "多次大内存分配释放测试通过" << std::endl;
    std::cout << "大内存mmap分配测试全部通过!" << std::endl;
}

int main() { test_memory_mmap(); }
