#pragma once
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/mman.h>

namespace xiaoyu {
    // mmap系统调用包装函数
    void* mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset) {
        return reinterpret_cast<void*>(syscall(SYS_mmap, addr, length, prot, flags, fd, offset));
    }

    // munmap系统调用包装函数
    int munmap(void* addr, size_t length) {
        return static_cast<int>(syscall(SYS_munmap, addr, length));
    }
}

#define m_mmap(x, y, z, a, b, c) xiaoyu::mmap(x, y, z, a, b, c)
#define m_munmap(x, y) xiaoyu::munmap(x, y)
