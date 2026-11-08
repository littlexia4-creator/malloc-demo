#pragma once
#include <cstdio>
#include <new>
#include "malloc.hpp"

// 重载全局new（单个对象）
void* operator new(std::size_t size) {
    void* ptr = xiaoyu::malloc(size);  // 使用malloc分配内存
    if (ptr == nullptr) {
        throw std::bad_alloc();  // 分配失败抛出异常
    }
    std::fprintf(stderr, "new size=%zu ptr=%p\n", size, ptr);
    return ptr;
}

// 重载全局delete（单个对象）
void operator delete(void* ptr) noexcept {
    if (ptr != nullptr) {
        std::fprintf(stderr, "delete ptr=%p\n", ptr);
        xiaoyu::free(ptr);  // 使用free释放内存
    }
}

// 重载全局new[]（数组）
void* operator new[](std::size_t size) {
    void* ptr = xiaoyu::malloc(size);
    if (ptr == nullptr) {
        throw std::bad_alloc();
    }
    std::fprintf(stderr, "new[] size=%zu ptr=%p\n", size, ptr);
    return ptr;
}

// 重载全局delete[]（数组）
void operator delete[](void* ptr) noexcept {
    if (ptr != nullptr) {
        std::fprintf(stderr, "delete[] ptr=%p\n", ptr);
        xiaoyu::free(ptr);
    }
}
