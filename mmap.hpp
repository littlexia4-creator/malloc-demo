#pragma once
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/mman.h>

namespace xiaoyu {
    // mmap系统调用包装函数
    inline void* mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset) {
        return reinterpret_cast<void*>(syscall(SYS_mmap, addr, length, prot, flags, fd, offset));
    }

    // mremap system call wrapper.
    inline void* mremap(void* addr, size_t old_size, size_t new_size, int flags) {
        return reinterpret_cast<void*>(syscall(SYS_mremap, addr, old_size, new_size, flags));
    }

    // munmap系统调用包装函数
    inline int munmap(void* addr, size_t length) {
        return static_cast<int>(syscall(SYS_munmap, addr, length));
    }
}

#define m_mmap(x, y, z, a, b, c) xiaoyu::mmap(x, y, z, a, b, c)
#define m_munmap(x, y) xiaoyu::munmap(x, y)
#define m_mremap(x, y, z, flags) xiaoyu::mremap(x, y, z, flags)
