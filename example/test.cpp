#include <iostream>
#include "ThreadPool.hpp"

int foo(int i) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return i;
}


int main() {
    thread_pool::ThreadPool pool {4};
    std::vector<std::future<int>> results;

    for (int i = 0; i < 100; ++i) {
        std::future<int> ret = pool.add_task(foo, i);
        results.push_back(std::move(ret));
    }

    int sum = 0;
    for (auto& result : results) {
        sum += result.get();
    }

    std::cout << "sum is " << sum << std::endl;

    return 0;
}