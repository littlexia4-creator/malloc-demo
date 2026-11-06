#include "common.hpp"

void print_mallinfo() {
    auto mallinfo = m_mallinfo2();
    std::cout << "mallinfo2:" << std::endl;
    std::cout << "  arena: " << mallinfo.arena << std::endl;
    std::cout << "  ordblks: " << mallinfo.ordblks << std::endl;
    std::cout << "  hblks: " << mallinfo.hblks << std::endl;
    std::cout << "  hblkhd: " << mallinfo.hblkhd << std::endl;
    std::cout << "  uordblks: " << mallinfo.uordblks << std::endl;
    std::cout << "  fordblks: " << mallinfo.fordblks << std::endl;
    std::cout << "  keepcost: " << mallinfo.keepcost << std::endl;
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

int main() { test_mallocinfo(); }
