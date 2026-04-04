#include "threadpool.h"

ThreadPool::ThreadPool(size_t num_threads){
    for(size_t i = 0; i < num_threads; ++i){
        
        workers_.emplace_back([this]{
            while(true){
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    cv_.wait(lock, [this]{return stop_ || !tasks_.empty();});
                    if(stop_ && tasks_.empty()){
                        return;
                    }
                    task = std::move(tasks_.front());
                    tasks_.pop();
                }
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool(){
    {
        std::unique_lock<std::mutex> lock(mutex_);
        stop_ = true;
    }
    cv_.notify_all();
    for(std::thread &worker : workers_){
        if(worker.joinable()){
            worker.join();
        }
    }
}

bool ThreadPool::Enqueue(std::function<void()> task){
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if(stop_){
            return false;
        }
        tasks_.emplace(std::move(task));
    }
    cv_.notify_one();
    return true;
}