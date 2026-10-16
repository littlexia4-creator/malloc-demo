#pragma once
#include <iostream>
#include "malloc.hpp"

// 重载全局new（单个对象）
void* operator new(std::size_t size) {
    void* ptr = xiaoyu::malloc(size);  // 使用malloc分配内存
    if (ptr == nullptr) {
        throw std::bad_alloc();  // 分配失败抛出异常
    }
    std::cout << "全局new分配 " << size << " 字节, ptr=" << ptr << std::endl;
    return ptr;
}

// 重载全局delete（单个对象）
void operator delete(void* ptr) noexcept {
    if (ptr != nullptr) {
        std::cout << "全局delete释放内存, ptr" << ptr << std::endl;
        xiaoyu::free(ptr);  // 使用free释放内存
    }
}

// 重载全局new[]（数组）
void* operator new[](std::size_t size) {
    void* ptr = xiaoyu::malloc(size);
    if (ptr == nullptr) {
        throw std::bad_alloc();
    }
    std::cout << "全局new[]分配 " << size << " 字节, ptr=" << ptr << std::endl;
    return ptr;
}

// 重载全局delete[]（数组）
void operator delete[](void* ptr) noexcept {
    if (ptr != nullptr) {
        std::cout << "全局delete[]释放内存, ptr=" << ptr << std::endl;
        xiaoyu::free(ptr);
    }
}