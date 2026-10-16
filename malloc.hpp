#pragma once
#include "sbrk.hpp"
#include "mmap.hpp"
#include <iostream>
#include <cstring>
#include <mutex>
#include "stddef.h"

namespace xiaoyu {
    // 64位内存块头部/尾部格式：
    // struct malloc_chunk
    // {
    //     uint32_t prev_footer;     // 前一个chunk大小 + 状态标志位
    //     uint32_t padding1; 
    //     uint32_t header; // 当前chunk大小 + 状态标志位
    //     uint32_t padding2;
    //     union {
    //         struct {
    //             malloc_chunk* fd;   // 空闲时：指向空闲链表上一个chunk
    //             malloc_chunk* bk;   // 空闲时：指向空闲链表下一个chunk
    //         };
    //         char user_data[0];            // 分配时：返回给用户的指针起始位置
    //     };
    // };

    // 32位内存块头部/尾部格式：
    // struct malloc_chunk
    // {
    //     uint32_t prev_footer;     // 前一个chunk大小 + 状态标志位
    //     uint32_t header; // 当前chunk大小 + 状态标志位
    //     union {
    //         struct {
    //             malloc_chunk* fd;   // 空闲时：指向空闲链表上一个chunk
    //             malloc_chunk* bk;   // 空闲时：指向空闲链表下一个chunk
    //         };
    //         char user_data[0];            // 分配时：返回给用户的指针起始位置
    //     };
    // };

    // 指针大小 4 字节或 8 字节
    #define PTR_SIZE sizeof(void*)
    // 内存块对齐大小 8字节或16字节
    #define MALLOC_ALIGNMENT (2*PTR_SIZE)

    /* The corresponding word size.  */
    #define SIZE_SZ (sizeof (size_t))

    /* The corresponding bit mask value.  */
    #define MALLOC_ALIGN_MASK (MALLOC_ALIGNMENT - 1)

    #define PAGE_SIZE 4096u
   
    // 块头部/尾部标记：使用最低3位作为标志位
    #define SIZE_MASK (~0x7)     // 大小掩码（清除标志位）
    #define ALLOCATED 0x1        // 块已分配标志
    #define MMAP_ALLOCATED 0x2   // 通过mmap分配的标志

    // 最小内存块大小 8字节16字节 (首尾块)
    #define MIN_BLOCK_SIZE MALLOC_ALIGNMENT
    // 空内存块大小 16字节或32字节 (非首尾块)
    #define EMPTY_BLOCK_SIZE (MIN_BLOCK_SIZE + 2*PTR_SIZE)

    #define M_TRIM_THRESHOLD    -1
    #define M_TOP_PAD           -2
    #define M_MMAP_THRESHOLD    -3
    #define M_MMAP_MAX          -4

    // 大内存分配阈值，超过此大小使用mmap分配
    #define DEFAULT_MMAP_THRESHOLD_MIN (128 * 1024)
    # if __WORDSIZE == 32
    #  define DEFAULT_MMAP_THRESHOLD_MAX (512 * 1024)
    # else
    #  define DEFAULT_MMAP_THRESHOLD_MAX (4 * 1024 * 1024 * sizeof(long))
    # endif

    #define DEFAULT_TOP_PAD        (0)

    #define DEFAULT_MMAP_MAX       (65536)

    // 堆内存回收阈值，当topChunk大小超过此值时考虑回收给系统
    #define DEFAULT_TRIM_THRESHOLD (128 * 1024)  // 128KB

    #define NBINS             128

    #define SMALLBIN_WIDTH  MALLOC_ALIGNMENT

    #define largebin_index_32(sz)                                                \
        (((((unsigned long)(sz)) >>  6) <= 38) ?  56 + (((unsigned long)(sz)) >>  6) :\
        ((((unsigned long)(sz)) >>  9) <= 20) ?  91 + (((unsigned long)(sz)) >>  9) :\
        ((((unsigned long)(sz)) >> 12) <= 10) ? 110 + (((unsigned long)(sz)) >> 12) :\
        ((((unsigned long)(sz)) >> 15) <=  4) ? 119 + (((unsigned long)(sz)) >> 15) :\
        ((((unsigned long)(sz)) >> 18) <=  2) ? 124 + (((unsigned long)(sz)) >> 18) :\
        126)

    #define largebin_index_64(sz)                                                \
        (((((unsigned long)(sz)) >>  6) <= 48) ?  48 + (((unsigned long)(sz)) >>  6) :\
        ((((unsigned long)(sz)) >>  9) <= 20) ?  91 + (((unsigned long)(sz)) >>  9) :\
        ((((unsigned long)(sz)) >> 12) <= 10) ? 110 + (((unsigned long)(sz)) >> 12) :\
        ((((unsigned long)(sz)) >> 15) <=  4) ? 119 + (((unsigned long)(sz)) >> 15) :\
        ((((unsigned long)(sz)) >> 18) <=  2) ? 124 + (((unsigned long)(sz)) >> 18) :\
        ((((unsigned long)(sz)) >> 22) <=  1) ? 126 + (((unsigned long)(sz)) >> 22) :\
        127)

    #define smallbin_index_64(sz) \
        (((unsigned)(sz)) >> 4)

    #define smallbin_index_32(sz) \
        (((unsigned)(sz)) >> 3)

    #define smallbin_index(sz) \
        ((SMALLBIN_WIDTH == 16) ? smallbin_index_64(sz) : \
        smallbin_index_32(sz))

    #define largebin_index(sz) \
        ((SMALLBIN_WIDTH == 16) ? largebin_index_64(sz) : \
        largebin_index_32(sz))

    #define bin_index(sz) \
        ((sz < 1024) ? smallbin_index(sz) : \
        largebin_index(sz))

    // 空闲链表指针访问宏
    #define GET_FD_FREE(block) (*(void **)((char *)(block) + 2*PTR_SIZE))
    #define GET_BK_FREE(block) (*(void **)((char *)(block) + 3*PTR_SIZE))
    #define SET_FD_FREE(block, fd) (GET_FD_FREE(block) = (fd))
    #define SET_BK_FREE(block, bk) (GET_BK_FREE(block) = (bk))

    #define MAX(a, b) ((a) > (b) ? (a) : (b))
    #define MIN(a, b) ((a) < (b) ? (a) : (b))

    typedef void* mchunkptr;

    struct malloc_par {
        size_t top_pad; // topChunk的最小填充大小
        size_t n_mmaps = 0;    // mmap分配的块数量
        size_t mmapped_mem = 0;   // mmap分配的总内存大小
        int n_mmaps_max;
        size_t mmap_threshold; // mmap分配阈值
        size_t trim_threshold; // 堆内存回收阈值
        bool no_dyn_threshold;

        /* Statistics */
        size_t max_n_mmaps;  // 最大mmap分配块数量
        size_t max_mmapped_mem; // 最大mmap分配总内存大小
    };

    // 全局变量
    struct malloc_par mp_= {
        .top_pad = DEFAULT_TOP_PAD,
        .n_mmaps_max = DEFAULT_MMAP_MAX,
        .mmap_threshold = DEFAULT_MMAP_THRESHOLD_MIN,
        .trim_threshold = DEFAULT_TRIM_THRESHOLD,
    };

    struct malloc_state {
        // std::mutex mutex; // 线程安全锁 TODO 待实现
        mchunkptr bins[NBINS] = {nullptr}; // 分配箱数组，每个bin管理特定大小范围的空闲块
        mchunkptr top = nullptr; // 指向当前堆的顶部空闲块（替代原heap_end）
        mchunkptr heap_start = nullptr;     // 堆起始地址
        // size_t attached_threads; // TODO 待实现多线程支持
    };
    
    static malloc_state main_arena;

    typedef malloc_state *mstate;

    struct heap_info
    {
        mstate ar_ptr; /* Arena for this heap. */
        heap_info *prev; /* Previous heap. */
        size_t size;   /* Current size in bytes. */
        size_t mprotect_size; /* Size in bytes that has been mprotected
                                PROT_READ|PROT_WRITE.  */
        size_t pagesize; /* Page size used when allocating the arena.  */
        /* Make sure the following data is properly aligned, particularly
            that sizeof (heap_info) + 2 * SIZE_SZ is a multiple of
            MALLOC_ALIGNMENT. */
        char pad[-3 * SIZE_SZ & MALLOC_ALIGN_MASK];
    } ;

    // 获取BLOCK偏移后的地址
    #define GET_BLOCK_OFFSET(block, offset) ((char*)(block) + (offset))
    // 获取BLOCK头部
    #define GET_BLOCK_HEADER(block) (*(int32_t*)GET_BLOCK_OFFSET(block, PTR_SIZE))
    // 获取上一个BLOCK脚部
    #define GET_BLOCK_PREV_FOOTER(block) (*(int32_t*)block)
    // 获取BLOCK大小
    #define GET_BLOCK_SIZE(block) (GET_BLOCK_HEADER(block) & SIZE_MASK)
    // 获取上一个BLOCK大小
    #define GET_PREV_BLOCK_SIZE(block) (GET_BLOCK_PREV_FOOTER(block) & SIZE_MASK)
    // 检查是否为mmap分配的块
    #define IS_MMAP_BLOCK(block) (GET_BLOCK_HEADER(block) & MMAP_ALLOCATED)
    // 获取BLOCK是否已分配
    #define IS_BLOCK_ALLOCATED(block) (GET_BLOCK_HEADER(block) & ALLOCATED)
    // 获取上一个BLOCK是否已分配
    #define IS_PREV_BLOCK_ALLOCATED(block) (GET_BLOCK_PREV_FOOTER(block) & ALLOCATED)
    // 获取下一个或上一个BLOCK
    #define GET_BLOCK_NEXT(block) GET_BLOCK_OFFSET(block, GET_BLOCK_SIZE(block))
    #define GET_BLOCK_PREV(block) GET_BLOCK_OFFSET(block, -GET_PREV_BLOCK_SIZE(block))
    // 获取BLOCK脚部
    #define GET_BLOCK_FOOTER(block) GET_BLOCK_PREV_FOOTER(GET_BLOCK_NEXT(block))
    // 访问已分配内存
    #define GET_DATA(block) GET_BLOCK_OFFSET(block, MALLOC_ALIGNMENT)
    // 获取已分配BLOCK
    #define GET_BLOCK(ptr) GET_BLOCK_OFFSET(ptr, -MALLOC_ALIGNMENT)

    /* Align a value by rounding down to closest size.
    e.g. Using size of 4096, we get this behavior:
        {4095, 4096, 4097} = {0, 4096, 4096}.  */
    #define ALIGN_DOWN(base, size)	((base) & -((__typeof__ (base)) (size)))

    /* Align a value by rounding up to closest size.
    e.g. Using size of 4096, we get this behavior:
        {4095, 4096, 4097} = {4096, 4096, 8192}.

    Note: The size argument has side effects (expanded multiple times).  */
    #define ALIGN_UP(base, size)	ALIGN_DOWN ((base) + (size) - 1, (size))

    static int do_set_trim_threshold (size_t value) {
        mp_.trim_threshold = value;
        mp_.no_dyn_threshold = 1;
        return 1;
    }

    static int do_set_mmap_threshold (size_t value) {
        mp_.mmap_threshold = value;
        mp_.no_dyn_threshold = 1;
        return 1;
    }

    static int do_set_top_pad (size_t value)
    {
        mp_.top_pad = value;
        mp_.no_dyn_threshold = 1;
        return 1;
    }

    static int do_set_mmaps_max (int32_t value)
    {
        mp_.n_mmaps_max = value;
        mp_.no_dyn_threshold = 1;
        return 1;
    }

    int mallopt(int param_number, int value) {
        switch(param_number) {
            case M_TRIM_THRESHOLD:
                return do_set_trim_threshold (value);
            case M_TOP_PAD:
                return do_set_top_pad (value);
            case M_MMAP_THRESHOLD:
                return do_set_mmap_threshold (value);
            case M_MMAP_MAX:
                return do_set_mmaps_max (value);
        }

        return 0;
    }

    // 设置topChunk的头部
    static void set_top_chunk(void* block, size_t size) {
        if (size < MIN_BLOCK_SIZE) {
            std::cerr << "块大小不能小于16字节" << std::endl;
            exit(1);
        }

        if (size < 0) {
            std::cerr << "块大小不能小于0" << std::endl;
            exit(1);
        }

        main_arena.top = block;
        GET_BLOCK_HEADER(main_arena.top) = size; // 设置头部
    }

    // 设置块头部和脚部
    static void set_block_header_footer(void *block, size_t size, bool allocated, bool is_mmap = false) {
        if (size < MIN_BLOCK_SIZE) {
            std::cerr << "块大小不能小于16字节" << std::endl;
            exit(1);
        }

        const size_t header = size | (is_mmap ? MMAP_ALLOCATED : 0) | (allocated ? ALLOCATED : 0);
        GET_BLOCK_HEADER(block) = header; // 设置头部
        if (!is_mmap) GET_BLOCK_FOOTER(block) = header; // 设置脚部
    }

    // 从topChunk分配内存
    static void* allocate_from_topchunk(size_t size) {
        if (size%MALLOC_ALIGNMENT != 0) {
            std::cerr << "分配大小必须是对齐的，当前大小：" << size << std::endl;
            exit(1); // 终止程序
        }
        auto topChunkSize = GET_BLOCK_SIZE(main_arena.top);

        if (topChunkSize < size + MIN_BLOCK_SIZE) {
            return nullptr;
        }
        
        // 设置分配块的头部和脚部
        void* allocated_block = main_arena.top;
        set_block_header_footer(allocated_block, size, true);
        
        // 更新topChunk位置
        set_top_chunk(GET_BLOCK_OFFSET(main_arena.top, size), topChunkSize - size);
        
        return allocated_block;
    }

    // 扩展topChunk（使用sbrk）
    static bool expand_topchunk(size_t request_size) {
        if (mp_.top_pad < 0) { // top_pad为负数会造成程序异常
            return false;
        }

        // 至少扩展到top_pad大小
        request_size = MAX((long)request_size, (long)(mp_.top_pad - GET_BLOCK_SIZE(main_arena.top)));
        // request_size 对齐到 PAGE_SIZE
        request_size = ALIGN_UP(request_size, PAGE_SIZE);
        
        if (request_size < 0){ // 防止top_pad很大而导致 request_size 变成0
            return false;
        }
        void* new_heap = m_sbrk(request_size);
        if (new_heap == (void*)-1) {
            return false; // sbrk失败
        }
        
        // 更新topChunk的大小
        set_top_chunk(main_arena.top, request_size + GET_BLOCK_SIZE(main_arena.top));

        return true;
    }

    // trim topChunk，将过大的内存归还给系统
    static size_t systrim(size_t pad) {
        if (main_arena.top == nullptr) {
            return 0;
        }

        size_t top_size = GET_BLOCK_SIZE(main_arena.top);
        if (top_size < mp_.trim_threshold) {
            return 0; // 不需要trim
        }
        if (top_size <= pad)
            return 0; // 不需要trim

        // 计算可释放的内存，但保留pad大小的缓冲
        size_t top_area = ALIGN_DOWN(top_size - MIN_BLOCK_SIZE, PAGE_SIZE);

        size_t amount_to_trim = ALIGN_DOWN(top_size - pad, PAGE_SIZE);

        // Ensure we don't trim more than the maximum page-aligned amount available
        amount_to_trim = MIN(amount_to_trim, top_area);
        
        if (amount_to_trim == 0) {
            return 0; // topChunk太小，不需要trim
        }

        // 使用brk回收内存（相当于sbrk的负值）
        void* new_brk = m_sbrk(-(ptrdiff_t)amount_to_trim);
        if (new_brk == (void*)-1) {
            return 0; // trim失败
        }

        // 更新topChunk大小
        set_top_chunk(main_arena.top, top_size - amount_to_trim);

        return amount_to_trim;
    }
   
    // 获取bin头部
    static void* get_bin_head(int bin_idx) {
        if (bin_idx < 2 or bin_idx >= NBINS){
            std::cerr << "无效的bin索引：" << bin_idx << std::endl;
            exit(1);
        }
        return main_arena.bins[bin_idx];
    }
    
    // 检查bin是否为空
    static bool is_bin_empty(int bin_idx) {
        if (bin_idx < 2 or bin_idx >= NBINS){
            std::cerr << "无效的bin索引：" << bin_idx << std::endl;
            exit(1);
        }
        return main_arena.bins[bin_idx] == nullptr;
    }
    
    // 从指定bin的空闲链表中移除块
    static void remove_from_bin(void *block) {
        int bin_idx = bin_index(GET_BLOCK_SIZE(block));

        if (bin_idx < 2 || bin_idx >= NBINS) {
            std::cerr << "无效的bin索引：" << bin_idx << std::endl;
            exit(1);
        }

        void *fd = GET_FD_FREE(block);
        void *bk = GET_BK_FREE(block);
        
        // 如果是bin中唯一的块
        if (fd == block and bk == block) {
            main_arena.bins[bin_idx] = nullptr;
        } else {
            // 更新前后块的指针
            SET_BK_FREE(fd, bk);
            SET_FD_FREE(bk, fd);
            
            // 如果移除的是bin头部，更新bin头部
            if (main_arena.bins[bin_idx] == block) {
                main_arena.bins[bin_idx] = fd;
            }
        }
    }
   
    // 将块添加到指定bin的空闲链表头部
    static void add_to_bin(void *block) {
        int bin_idx = bin_index(GET_BLOCK_SIZE(block));

        if (bin_idx < 2 or bin_idx >= NBINS) {
            std::cerr << "无效的bin索引：" << bin_idx << std::endl;
            exit(1);
        }
        
        if (main_arena.bins[bin_idx] == nullptr) {
            // bin为空，创建单独的循环链表
            main_arena.bins[bin_idx] = block;
            SET_FD_FREE(block, block);
            SET_BK_FREE(block, block);
        } else {
            // bin不为空，插入到头部
            void *head = main_arena.bins[bin_idx];
            void *bk = GET_BK_FREE(head);
            
            SET_BK_FREE(head, block);
            SET_FD_FREE(block, head);
            SET_BK_FREE(block, bk);
            SET_FD_FREE(bk, block);
            
            main_arena.bins[bin_idx] = block; // 更新bin头部
        }
    }
   
    // 合并相邻空闲块
    static void *coalesce(void *block) {
        if (IS_BLOCK_ALLOCATED(block)) {
            std::cerr << "无法合并已分配的块。" << std::endl;
            exit(1);
        }

        size_t size = GET_BLOCK_SIZE(block);
   
        // 合并后一个块
        void *next_block = GET_BLOCK_NEXT(block);
        bool next_allocated = IS_BLOCK_ALLOCATED(next_block);
        if (!next_allocated and next_block != main_arena.top) {
            remove_from_bin(next_block);
            size += GET_BLOCK_SIZE(next_block);
        }

        // 合并前一个块
        bool prev_allocated = IS_PREV_BLOCK_ALLOCATED(block);
        if (!prev_allocated) {
            void * prev_block = GET_BLOCK_PREV(block);
            remove_from_bin(prev_block);
            size += GET_BLOCK_SIZE(prev_block);
            block = prev_block;
        }
   
        // 设置合并后的块头部和脚部
        set_block_header_footer(block, size, false);
        return block;
    }
   
    // 初始化堆
    static void init_heap() {
        auto heap_top = m_sbrk(0);
        // assert(heap_top 对齐到 PAGE_SIZE)
        if (((uintptr_t)heap_top) % PAGE_SIZE != 0) {
            std::cerr << "堆起始地址未对齐到页大小：" << PAGE_SIZE << std::endl;
            exit(1);
        }
        auto requested_size = PAGE_SIZE;
        void *heap = m_sbrk(requested_size);
        if (heap == (void *)-1) return;

        // 初始化bins数组
        memset(main_arena.bins, 0, sizeof(main_arena.bins));
   
        // 初始化堆首块
        main_arena.heap_start = heap;
        set_block_header_footer(main_arena.heap_start, MIN_BLOCK_SIZE, true); // 首块设为已分配，16字节大小

        // 初始化topChunk
        set_top_chunk(GET_BLOCK_NEXT(main_arena.heap_start), PAGE_SIZE-MIN_BLOCK_SIZE); // topChunk设为未分配，(4096-16)字节大小
    }
   
    void free_internal(void *block) {
        // 标记为空闲
        set_block_header_footer(block, GET_BLOCK_SIZE(block), false);

        // 合并相邻的空闲块
        void *merged_block = coalesce(block);
       
        // 检查合并后的块是否与topChunk相邻
        if (GET_BLOCK_NEXT(merged_block) == main_arena.top) {
            // 与topChunk相邻，将其合并到topChunk中
            auto new_size = GET_BLOCK_SIZE(merged_block) + GET_BLOCK_SIZE(main_arena.top);
            set_top_chunk(merged_block, new_size);
            
            // 合并到topChunk后，检查是否需要trim
            size_t trimmed = systrim(mp_.top_pad); // 使用top_pad大小进行trim
        } else {
            // 不与topChunk相邻，将合并后的块加入对应的bin
            add_to_bin(merged_block);
        }
    }

    // mmap分配大内存块
    static void* sysmalloc_mmap(size_t size) {
        // 计算需要分配的总大小（包括头部、脚部）
        size_t total_size = size + MIN_BLOCK_SIZE;
        // 页对齐
        total_size = (total_size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
       
        // 使用mmap分配内存
        void* addr = m_mmap(nullptr, total_size,
                                  PROT_READ | PROT_WRITE,
                                  MAP_PRIVATE | MAP_ANONYMOUS,
                                  -1, 0);
       
        if (addr == MAP_FAILED) {
            return nullptr;
        }
       
        // 设置块头部和脚部，标记为mmap分配
        set_block_header_footer(addr, total_size, true, true);
        ++mp_.n_mmaps;
        mp_.max_n_mmaps = std::max(mp_.max_n_mmaps, mp_.n_mmaps);
        mp_.mmapped_mem += total_size;
        mp_.max_mmapped_mem = std::max(mp_.max_mmapped_mem, mp_.mmapped_mem);
        return GET_DATA(addr);
    }

    // 释放mmap分配的内存块
    static void mmap_free(void* block) {
        size_t total_size = GET_BLOCK_SIZE(block);
       
        if (m_munmap(block, total_size) != 0) {
            std::cerr << "munmap失败" << std::endl;
            exit(1);
        }
        --mp_.n_mmaps;
        mp_.mmapped_mem -= total_size;
    }

    // malloc函数实现
    void *malloc(size_t size) {
        // 检查是否需要使用mmap分配大内存
        if (size >= mp_.mmap_threshold and mp_.mmapped_mem < mp_.n_mmaps_max) {
            return sysmalloc_mmap(size);
        }

        // 初始化堆（如果未初始化）
        if (main_arena.heap_start == nullptr) init_heap();
       
        if (main_arena.heap_start == nullptr or main_arena.top == nullptr) {
            std::cerr << "堆初始化失败，无法分配内存。" << std::endl;
            exit(1);
        }

        // 计算需要分配的总大小（包括头部、脚部）
        size_t total_size = size + MIN_BLOCK_SIZE;
        total_size = MAX(total_size, EMPTY_BLOCK_SIZE);
        total_size = ALIGN_UP(total_size, MALLOC_ALIGNMENT);  // 对齐到ALIGNMENT

        // 在bins中查找合适的块
        void *best_fit = nullptr;
        int target_bin_idx = bin_index(total_size);
        
        // 先在目标bin中查找精确或最优匹配
        if (!is_bin_empty(target_bin_idx)) {
            void *current = main_arena.bins[target_bin_idx];
            void *start = current;
            do {
                if (GET_BLOCK_SIZE(current) >= total_size) {
                    best_fit = current;
                    break;
                }
                current = GET_FD_FREE(current);
            } while (current != start);
        }
        
        // 如果目标bin中没有找到，在更大的bins中查找
        if (best_fit == nullptr) {
            for (int i = target_bin_idx + 1; i < NBINS; i++) {
                if (!is_bin_empty(i)) {
                    best_fit = main_arena.bins[i]; // 使用第一个可用的块
                    break;
                }
            }
        }

        // 处理从bins中找到的块
        if (best_fit != nullptr) {
            if (IS_BLOCK_ALLOCATED(best_fit)) {
                std::cerr << "分配失败：找到的块已被分配。" << std::endl;
                exit(1);
            }

            // 从找到的块中分配内存
            size_t available_size = GET_BLOCK_SIZE(best_fit);
            size_t remaining_size = available_size - total_size;
           
            // 从对应的bin中移除已分配块
            remove_from_bin(best_fit);

            // 检查剩余空间是否足够作为独立空闲块
            if (remaining_size >= EMPTY_BLOCK_SIZE) {
                // 分割块
                set_block_header_footer(best_fit, total_size, true);
                void *new_free_block = GET_BLOCK_OFFSET(best_fit, total_size);
                // 设置新块大小
                set_block_header_footer(new_free_block, remaining_size, false);
                free_internal(new_free_block);
            } else {
                // 整个块分配
                set_block_header_footer(best_fit, available_size, true);
            }
           
            // 返回数据区域的起始地址（跳过头部）
            return GET_DATA(best_fit);
        }

        // 如果bins中都没有找到合适的块，尝试使用topChunk
        if (best_fit == nullptr) {
            auto need_size = total_size + MIN_BLOCK_SIZE;
            if (GET_BLOCK_SIZE(main_arena.top) >= need_size or expand_topchunk(need_size - GET_BLOCK_SIZE(main_arena.top))) {
                // 从topChunk分配
                void* allocated_block = allocate_from_topchunk(total_size);
                if (allocated_block != nullptr) {
                    return GET_DATA(allocated_block);
                }
            }
        }

        // 如果到这里说明所有分配尝试都失败了
        return nullptr;
    }
   
    // free函数实现
    void free(void *ptr) {
        if (ptr == nullptr) return;

        // 获取块起始地址（数据区域前8字节）
        void *block = GET_BLOCK(ptr);
       
        // 检查是否为mmap分配的块
        if (IS_MMAP_BLOCK(block)) {

            if (!mp_.no_dyn_threshold
                and GET_BLOCK_SIZE(block) > mp_.mmap_threshold
                and GET_BLOCK_SIZE(block) <= DEFAULT_MMAP_THRESHOLD_MAX)
            {
                mp_.mmap_threshold = GET_BLOCK_SIZE(block);
                mp_.trim_threshold = 2 * mp_.mmap_threshold;
            }

            mmap_free(block);
        } else {
            free_internal(block);
        }
    }

    int malloc_trim(size_t pad) {
        if (main_arena.top == nullptr) {
            return 0;
        }

        // pad参数表示保留给topChunk的最小大小，超过这个大小的部分会被trim
        size_t old_trim_threshold = mp_.trim_threshold;
        
        // 临时设置trim阈值为pad，然后执行trim
        mp_.trim_threshold = pad;
        size_t trimmed = systrim(mp_.top_pad); // 使用top_pad大小进行trim

        // 恢复原来的trim阈值
        mp_.trim_threshold = old_trim_threshold;
        return 0;
    }

    std::pair<size_t,size_t> get_free_blocks_info(){
        // 检查bins中是否还有未归还内存的块
        size_t free_blocks_cnt = 0;
        size_t free_blocks_total_size = 0;
        for (int i = 2; i < NBINS; i++) {
            if (main_arena.bins[i] != nullptr) {
                void *current = main_arena.bins[i];
                void *start = current;
                do {
                    free_blocks_cnt++;
                    free_blocks_total_size += GET_BLOCK_SIZE(current);
                    current = GET_FD_FREE(current);
                } while (current != start);
            }
        }

        return {free_blocks_cnt, free_blocks_total_size};
    }

    struct mallinfo2
    {
        size_t arena;    /* non-mmapped space allocated from system */
        size_t ordblks;  /* number of free chunks */
        size_t hblks;    /* number of mmapped regions */
        size_t hblkhd;   /* space in mmapped regions */
        size_t uordblks; /* total allocated space */
        size_t fordblks; /* total free space */
        size_t keepcost; /* top-most, releasable (via malloc_trim) space */
    };

    mallinfo2 get_mallinfo2() {
        auto [free_blocks_cnt, free_blocks_total_size] = get_free_blocks_info();
        mallinfo2 ans;
        ans.hblks = mp_.n_mmaps; // mmap分配的块数量
        ans.hblkhd = mp_.mmapped_mem; // mmap分配的总内存大小
        ans.keepcost = main_arena.top?GET_BLOCK_SIZE(main_arena.top):0;
        ans.ordblks = free_blocks_cnt;
        ans.fordblks = ans.keepcost + free_blocks_total_size;
        ans.arena = (uintptr_t)main_arena.top - (uintptr_t)main_arena.heap_start + ans.keepcost;
        ans.uordblks = ans.arena - ans.fordblks;
        return ans;
    }

    void end_callback() {
        auto mallinfo = get_mallinfo2();
        if (mallinfo.uordblks !=0 and mallinfo.uordblks != MIN_BLOCK_SIZE) {
            std::cerr << "存在未释放的内存，已分配总额=" << mallinfo.uordblks << " 字节" << std::endl;
            return;
        }

        if (mp_.trim_threshold - mp_.top_pad >= PAGE_SIZE and mallinfo.keepcost >= mp_.trim_threshold) {
            std::cerr << "topChunk大于trim_threshold，存在内存没有按照预期归还, keepcost=" << mallinfo.keepcost << " 字节, 期望小于" << mp_.trim_threshold << " 字节" << std::endl;
            return;
        }

        if (mp_.mmap_threshold > mp_.top_pad) {
            if (mp_.top_pad!=-1 and mallinfo.keepcost < mp_.top_pad) {
                std::cerr << "topChunk小于top_pad，存在内存没有按照预期归还, keepcost=" << mallinfo.keepcost << " 字节, 期望大于等于" << mp_.top_pad << " 字节" << std::endl;
                return;
            }
        }else if (mp_.mmap_threshold <= mp_.top_pad) {
            // pass
        }

        // 检查bins中是否还有未归还内存的块
        if (mallinfo.ordblks) {
            std::cerr << "bins中有空闲块未归还系统，数量=" << mallinfo.ordblks << std::endl;
            return;
        }

        // 检查bins中是否还有未归还内存的块
        if (mallinfo.fordblks != mallinfo.keepcost) {
            std::cerr << "bins中有空闲块未归还系统，总大小=" << mallinfo.fordblks - mallinfo.keepcost << std::endl;
            return;
        }

        if(mallinfo.hblks or mallinfo.hblkhd) {
            std::cerr << "存在未释放的mmap分配内存块, mmap分配计数=" << mallinfo.hblks << ", mmap分配总额=" << mallinfo.hblkhd << std::endl;
            return ;
        }

        std::cout <<"mmap_threshold=" << mp_.mmap_threshold << ", trim_threshold=" << mp_.trim_threshold << ", top_pad=" << mp_.top_pad << std::endl;
        std::cout <<"max_n_mmaps=" << mp_.max_n_mmaps << ", max_mmapped_mem=" << mp_.max_mmapped_mem << std::endl;
        std::cout << "内存管理结束回调函数执行完毕。" << std::endl;
    }
}