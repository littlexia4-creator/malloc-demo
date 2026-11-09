#pragma once
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <vector>
#ifdef TEST_USE_SYSTEM_MALLOC
#include <malloc.h>
#define m_malloc(x) std::malloc(x)
#define m_realloc(x, y) std::realloc(x, y)
#define m_free(x) std::free(x)
#define m_mallopt(x, y) ::mallopt(x, y)
#define m_malloc_trim(x) ::malloc_trim(x)
#define m_mallinfo2() ::mallinfo2()
#define m_malloc_usable_size(x) ::malloc_usable_size(x)
#else
#include "../malloc.hpp"
#define m_malloc(x) xiaoyu::malloc(x)
#define m_realloc(x, y) xiaoyu::realloc(x, y)
#define m_free(x) xiaoyu::free(x)
#define m_mallopt(x, y) xiaoyu::mallopt(x, y)
#define m_malloc_trim(x) xiaoyu::malloc_trim(x)
#define m_mallinfo2() xiaoyu::get_mallinfo2()
#define m_malloc_usable_size(x) xiaoyu::malloc_usable_size(x)
#endif
