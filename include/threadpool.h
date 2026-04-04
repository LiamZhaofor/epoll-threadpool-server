#pragma once

#include <functional>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <cstddef>


class ThreadPool {
    public:
        ThreadPool(size_t num_threads);
        ~ThreadPool();
        bool Enqueue(std::function<void()> task);


    private:
        std::vector<std::thread> workers_;
        std::queue<std::function<void()>> tasks_;
        std::mutex mutex_;
        std::condition_variable cv_;
        bool stop_{false};
};
