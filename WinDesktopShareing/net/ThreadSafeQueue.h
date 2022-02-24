#ifndef THREAD_SAFE_QUEUE_H
#define THREAD_SAFE_QUEUE_H

#include <mutex>
#include <thread>
#include <queue>

namespace xop {
    template<typename T>
    class ThreadSafeQueue {
    public:
        ThreadSafeQueue()
        {

        }

        ThreadSafeQueue(ThreadSafeQueue const &other)
        {
            LG lg(mutex_);
            data_queue_ = other.data_queue_;
        }

        ~ThreadSafeQueue()
        {

        };

        void Push(T value)
        {

        }

        bool WaitAndPop(T &value)
        {
            std::unique_lock<std::mutex> lock(mutex_);
        }


    private:
        using LG = std::lock_guard<std::mutex>;
        std::mutex mutex_;
        std::queue<T> data_queue_;
        std::condition_variable data_cond_;
    };
}

#endif
