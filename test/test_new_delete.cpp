#include "common.hpp"
#ifndef TEST_USE_SYSTEM_MALLOC
#include "../new.hpp"
#endif

void test_new() {
    class MyClass {
    public:
        MyClass() {
            std::cout << "MyClass constructor called, this=" << this << std::endl;
        }
        int value1;
        int value2[10];
        void *value4;
    };

    // 测试单个对象
    int* p = new int(10);  // 调用全局new
    delete p;              // 调用全局delete

    // 测试数组
    int* arr = new int[5];  // 调用全局new[]
    delete[] arr;           // 调用全局delete[]

    auto ptr = new MyClass[5]; // 测试自定义类的new[]
    delete[] ptr;              // 测试自定义类的delete[]
}

int main() { test_new(); }
