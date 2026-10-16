#pragma once

#include <sys/syscall.h>
#include <unistd.h>
#include <cstdint>

namespace xiaoyu {
    void* brk(void* addr) {
        return reinterpret_cast<void*>(syscall(SYS_brk, addr));
    }

    // 自定义sbrk函数实现
    void* sbrk(intptr_t increment) {
        // 获取当前程序断点
        static void* current_brk = xiaoyu::brk(0);

        if (increment == 0) {
            return current_brk;
        }
   
        // 计算新的断点位置
        void* new_brk = reinterpret_cast<void*>(
            reinterpret_cast<uintptr_t>(current_brk) + increment
        );
   
        // 设置新的断点
        void* result = xiaoyu::brk(new_brk);
   
        // 检查是否成功设置
        if (result == new_brk) {
            auto* old_brk = current_brk;  // 保存原来的断点位置
            current_brk = new_brk;  // 更新当前断点
            return old_brk;
        } else {
            return reinterpret_cast<void*>(-1);  // 失败返回-1
        }
    }
}

#define m_sbrk(x) xiaoyu::sbrk(x)
