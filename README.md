# thread_pool
C++23 light weight thread_pool
head file only
# build
+ require: cmake 3.11(or later) g++13
+ make build && cmake .. && make -j
# Example
```cpp
#include <iostream>

#include "thread_pool.h"

int NormalFunc(int val) noexcept {
    std::cout << "call normal func\n";
    return val;
}

struct CallObject
{
    int MemberFunc(int val) noexcept { 
        std::cout << "call member func\n";
        return val;
    }
};

int main() {
    //caution: callfunc must be noexcept
    //init four thread
    util::ThreadPool<> thread_pool{4, []() noexcept {
        std::cout << " thread init \n";
    }};

    //push lambda
    thread_pool.PostDetach([] noexcept {
        std::cout << "call lambda PostDetach\n";
    });

    //get async result
    auto future = thread_pool.Post([] noexcept {
        std::cout << "call lambda Post and return result\n";
        return 1;
    });
    //wait result
    std::cout << "get async result " << future.get() << std::endl;

    //execute normal func
    future = thread_pool.Post(NormalFunc, 1);
    std::cout << "get normal func result " << future.get() << std::endl;

    //execute member func
    auto callobject = CallObject{};
    future = thread_pool.Post(&CallObject::MemberFunc, &callobject, 1);
    std::cout << "get member func result " << future.get() << std::endl;

    return 0;
}
```
