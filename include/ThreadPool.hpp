/**
 * @file ThreadPool.hpp
 * @author JiWei
 * @date 01/24/2024
 * @brief A simple implementation of thread pool
 * @cite https://github.com/progschj/ThreadPool
*/

#ifndef THREAD_POOL_HPP_
#define THREAD_POOL_HPP_

#include <vector>
#include <thread>
#include <queue>
#include <atomic>
#include <functional>
#include <mutex>
#include <future>
#include <condition_variable>
#include <type_traits>

namespace thread_pool {

class ThreadPool {
public:
    /**
     * @brief constructor
     * @param[in] _number: number of threads in pool
    */
    ThreadPool(size_t _number);

    /**
     * @brief destructor
    */
    ~ThreadPool();

    /**
     * @brief add task
    */
    template<typename F, typename... Args>
    auto add_task(F&& f, Args&&... args) 
        -> std::future<decltype(f(args...))>;

private:
    std::atomic_bool running_;
    std::size_t number_;
    std::vector<std::thread> workers_; 
    std::mutex mutex_;
    std::condition_variable condition_;
    std::queue<std::function<void()>> tasks_;
};


ThreadPool::ThreadPool(size_t _number) :
        running_(true),
        number_(_number) {
    for (size_t i = 0; i < number_; ++i) {
        workers_.push_back(std::thread(
            [&] {
                while (true) {
                    std::function<void()> task;
                    std::unique_lock<std::mutex> its_lock(mutex_);
                    while (running_ && tasks_.empty()) {
                        condition_.wait(its_lock);
                    }
                    if (!running_) {
                        break;
                    }
                    task = std::move(tasks_.front());
                    tasks_.pop();
                    its_lock.unlock();
                    task();
                }
            }
        ));
    }
}


ThreadPool::~ThreadPool() {
    running_ = false;
    condition_.notify_all();
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

template<typename F, typename... Args>
auto ThreadPool::add_task(F&& f, Args&&... args) 
        -> std::future<decltype(f(args...))> {
    using ret_type = decltype(f(args...));
    auto task = std::make_shared<std::packaged_task<ret_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    std::future<ret_type> ret = task->get_future();
    {
        std::lock_guard<std::mutex> its_lock(mutex_);
        tasks_.emplace([task] () {(*task)();});
    }
    condition_.notify_one();
    return ret;
}

}   // namespace thread_pool

#endif  // THREAD_POOL_HPP_