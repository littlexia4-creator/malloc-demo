#include "common.hpp"
#include "../malloc.hpp"

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

int main() { show_bin_index(); }
