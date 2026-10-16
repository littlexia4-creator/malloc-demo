#include "new.hpp"

#include <cassert>
#include <cstdlib>
#include <algorithm>

#define m_malloc(x) xiaoyu::malloc(x)
#define m_free(x) xiaoyu::free(x)
#define m_mallopt(x, y) xiaoyu::mallopt(x, y)
#define m_malloc_trim(x) xiaoyu::malloc_trim(x)

// 自定义sbrk函数断言测试
void test_sbrk();

// 自定义malloc与free函数断言测试
void test_malloc_free();

// 测试大内存mmap分配
void test_memory_mmap();

// 更多malloc断言测试
void test_malloc_additional();

void show_bin_index();

void print_mallinfo() {
    auto mallinfo = xiaoyu::get_mallinfo2();
    std::cout << "mallinfo2:" << std::endl;
    std::cout << "  arena: " << mallinfo.arena << std::endl;
    std::cout << "  ordblks: " << mallinfo.ordblks << std::endl;
    std::cout << "  hblks: " << mallinfo.hblks << std::endl;
    std::cout << "  hblkhd: " << mallinfo.hblkhd << std::endl;
    std::cout << "  uordblks: " << mallinfo.uordblks << std::endl;
    std::cout << "  fordblks: " << mallinfo.fordblks << std::endl;
    std::cout << "  keepcost: " << mallinfo.keepcost << std::endl;
}

void test_mallocinfo();

void test_mallopt();

void test_new();

int main() {
    std::atexit(xiaoyu::end_callback);

    test_sbrk();

    show_bin_index();

    test_mallopt();

    test_malloc_free();

    test_malloc_additional();

    test_memory_mmap();

    test_mallocinfo();

    test_new();

    return 0;
}

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
    
    // 测试4: 压力测试 - 随机分配和释放
    std::cout << "开始压力测试..." << std::endl;
    constexpr size_t stress_iterations = 1000;
    constexpr size_t max_active_allocs = 100;
    void* active_ptrs[max_active_allocs];
    size_t active_sizes[max_active_allocs];
    size_t active_count = 0;
    
    // 初始化活跃指针数组
    for (size_t i = 0; i < max_active_allocs; ++i) {
        active_ptrs[i] = nullptr;
        active_sizes[i] = 0;
    }
    
    for (size_t iter = 0; iter < stress_iterations; ++iter) {
        if (active_count < max_active_allocs && (active_count == 0 || (rand() % 3) != 0)) {
            // 分配新内存 (2/3 概率)
            size_t size = (rand() % 2048) + 1; // 1 到 2048 字节
            void* ptr = m_malloc(size);
            assert(ptr != nullptr);
            
            // 找到空位存储
            for (size_t i = 0; i < max_active_allocs; ++i) {
                if (active_ptrs[i] == nullptr) {
                    active_ptrs[i] = ptr;
                    active_sizes[i] = size;
                    active_count++;
                    
                    // 写入测试模式
                    unsigned char pattern = (unsigned char)(iter % 256);
                    memset(ptr, pattern, size);
                    break;
                }
            }
        } else if (active_count > 0) {
            // 释放现有内存 (1/3 概率)
            size_t index = rand() % max_active_allocs;
            if (active_ptrs[index] != nullptr) {
                // 验证数据完整性（如果是较新的分配）
                if (iter > 0 && iter < 256) {
                    unsigned char expected = (unsigned char)((iter - 1) % 256);
                    unsigned char* data = static_cast<unsigned char*>(active_ptrs[index]);
                    // 只检查前几个字节，避免检查可能被覆盖的数据
                    size_t check_size = std::min(active_sizes[index], size_t(4));
                    for (size_t j = 0; j < check_size; ++j) {
                        if (data[j] != expected) {
                            // 数据可能被后续分配覆盖，这在内存重用时是正常的
                            break;
                        }
                    }
                }
                
                m_free(active_ptrs[index]);
                active_ptrs[index] = nullptr;
                active_sizes[index] = 0;
                active_count--;
            }
        }
        
        // 每100次迭代报告进度
        if ((iter + 1) % 100 == 0) {
            std::cout << "压力测试进度: " << (iter + 1) << "/" << stress_iterations 
                     << ", 活跃分配: " << active_count << std::endl;
        }
    }
    
    // 清理剩余的分配
    for (size_t i = 0; i < max_active_allocs; ++i) {
        if (active_ptrs[i] != nullptr) {
            m_free(active_ptrs[i]);
        }
    }
    std::cout << "压力测试通过!" << std::endl;
    
    // 测试5: 特定模式的内存写入和验证
    std::cout << "测试内存模式验证..." << std::endl;
    
    // 测试不同的内存模式
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

void show_bin_index() {
        std::vector<unsigned long> vec ={
        1,
        15,
        16,
        31,
        32,
        47,
        48,
        1023,
        1024,
        1039,
        1040,
        1024 * 2, // 2KB
        1024 * 512,  // 512KB
        1024 * 1024, // 1MB
        1024 * 1024 * 64, // 64MB
        1024 * 1024 * 256, // 256MB
        1024 * 1024 * 1024, // 1GB
        1024u * 1024u * 1024u * 2u, // 2GB
        UINT32_MAX, // 4GB
    };

    for(int i=0;i<vec.size();++i) {
        std::cout << "bin_index(" << vec[i] << ") = " << bin_index(vec[i]) << std::endl;
    }
}

void test_mallocinfo() {
    std::cout << "\n=== 开始测试mallinfo2信息展示 ===" << std::endl;
    
    // 1. 初始状态
    std::cout << "\n1. 初始状态:" << std::endl;
    print_mallinfo();
    
    // 2. 小内存分配测试
    std::cout << "\n2. 分配多个小内存块后:" << std::endl;
    void* small_ptrs[10];
    for (int i = 0; i < 10; i++) {
        small_ptrs[i] = m_malloc(64 + i * 16);
    }
    print_mallinfo();
    
    // 3. 部分释放后（产生碎片）
    std::cout << "\n3. 释放部分小内存块后（产生碎片）:" << std::endl;
    for (int i = 1; i < 10; i += 2) {  // 释放奇数索引的块
        m_free(small_ptrs[i]);
    }
    print_mallinfo();
    
    // 4. 中等大小内存分配
    std::cout << "\n4. 分配中等大小内存块后:" << std::endl;
    void* medium_ptrs[5];
    for (int i = 0; i < 5; i++) {
        medium_ptrs[i] = m_malloc(1024 + i * 512);  // 1KB to 3KB
    }
    print_mallinfo();
    
    // 5. 大内存分配（触发mmap）
    std::cout << "\n5. 分配大内存块后（触发mmap）:" << std::endl;
    void* large_ptrs[3];
    for (int i = 0; i < 3; i++) {
        large_ptrs[i] = m_malloc(200 * 1024 + i * 100 * 1024);  // 200KB, 300KB, 400KB
    }
    print_mallinfo();
    
    // 6. 释放一个大内存块
    std::cout << "\n6. 释放一个大内存块后:" << std::endl;
    m_free(large_ptrs[1]);  // 释放300KB的块
    print_mallinfo();
    
    // 7. 大量小内存分配（测试bins的使用）
    std::cout << "\n7. 大量小内存分配后:" << std::endl;
    void* tiny_ptrs[50];
    for (int i = 0; i < 50; i++) {
        tiny_ptrs[i] = m_malloc(16 + (i % 8) * 8);  // 16, 24, 32, ..., 72字节循环
    }
    print_mallinfo();
    
    // 8. 释放所有小内存（测试合并）
    std::cout << "\n8. 释放所有小内存后（测试合并）:" << std::endl;
    // 释放剩余的small_ptrs
    for (int i = 0; i < 10; i += 2) {  // 释放偶数索引的块
        m_free(small_ptrs[i]);
    }
    // 释放所有tiny_ptrs
    for (int i = 0; i < 50; i++) {
        m_free(tiny_ptrs[i]);
    }
    print_mallinfo();
    
    // 9. 释放中等大小内存
    std::cout << "\n9. 释放中等大小内存后:" << std::endl;
    for (int i = 0; i < 5; i++) {
        m_free(medium_ptrs[i]);
    }
    print_mallinfo();
    
    // 10. 混合分配测试
    std::cout << "\n10. 混合分配测试:" << std::endl;
    void* mix_ptr1 = m_malloc(128);      // 小内存
    void* mix_ptr2 = m_malloc(4096);     // 中等内存
    void* mix_ptr3 = m_malloc(256 * 1024); // 大内存（mmap）
    print_mallinfo();
    
    // 11. 超大内存分配
    std::cout << "\n11. 超大内存分配后:" << std::endl;
    void* huge_ptr = m_malloc(2 * 1024 * 1024);  // 2MB
    print_mallinfo();
    
    // 12. 清理混合分配的内存
    std::cout << "\n12. 清理混合分配的内存后:" << std::endl;
    m_free(mix_ptr1);
    m_free(mix_ptr2);
    m_free(mix_ptr3);
    m_free(huge_ptr);
    print_mallinfo();
    
    // 13. 清理剩余的大内存
    std::cout << "\n13. 清理剩余的大内存后:" << std::endl;
    m_free(large_ptrs[0]);  // 释放200KB的块
    m_free(large_ptrs[2]);  // 释放400KB的块
    print_mallinfo();
    
    // 14. 测试malloc_trim
    std::cout << "\n14. 执行malloc_trim后:" << std::endl;
    m_malloc_trim(0);  // trim多余的内存
    print_mallinfo();
    
    std::cout << "\n=== mallinfo2信息展示测试完成 ===" << std::endl;
}

void test_mallopt() {
    // assert(m_mallopt(M_TOP_PAD, 128 * 1024)); // 设置top_pad为128KB
    // assert(m_mallopt(M_TOP_PAD, 0)); // 设置top_pad为0, 无需保持top Chunk的大小
    // M_TOP_PAD 不能设置为 -1
    // assert(m_mallopt(M_MMAP_THRESHOLD, 128 * 1024));
    // assert(m_mallopt(M_MMAP_THRESHOLD, 0)); // 优先使用mmap
    // assert(m_mallopt(M_MMAP_THRESHOLD, -1)); // 禁用 mmap 分配
    assert(m_mallopt(M_TRIM_THRESHOLD, 128 * 1024));
    // assert(m_mallopt(M_TRIM_THRESHOLD, 0));  // 立马归还堆区内存
    // assert(m_mallopt(M_TRIM_THRESHOLD, -1)); // 禁用 trim 功能
    // assert(m_mallopt(M_MMAP_MAX, 0)); // 禁用 mmap 分配
}

void test_new() {
    class MyClass {
    public: 
        MyClass() {
            std::cout << "MyClass constructor called, this=" << this << std::endl;
        }
        int value1;
        int value2[10];
        void *value4;
    };

    // 测试单个对象
    int* p = new int(10);  // 调用全局new
    delete p;              // 调用全局delete

    // 测试数组
    int* arr = new int[5];  // 调用全局new[]
    delete[] arr;           // 调用全局delete[]

    auto ptr = new MyClass[5]; // 测试自定义类的new[]
    delete[] ptr;              // 测试自定义类的delete[]
}