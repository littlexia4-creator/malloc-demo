#include "common.hpp"
#include <sstream>
#ifndef TEST_USE_SYSTEM_MALLOC
#include "../new.hpp"
#endif

void test_new_delete_with_redirected_output() {
    // Writing to this stream may allocate, exposing recursive allocator logging.
    std::ostringstream output;
    auto* previous = std::cout.rdbuf(output.rdbuf());
    int* p = new int(42);
    assert(*p == 42);
    delete p;
    std::cout.rdbuf(previous);
}

int main() { test_new_delete_with_redirected_output(); }
