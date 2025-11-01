#pragma once
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

/*
 * ThreadPool
 * ----------
 * A simple fixed-size thread pool capped at 4 threads. Can always be changed btw, 
    just hard coded here for now
 * - Accepts jobs (std::function<void()>)
 * - Executes them concurrently on background threads
 * - Uses condition variables for synchronization
 *
 * Avoiding this "std::thread::hardware_concurrency()" since it will be laggy :"D
 */
class ThreadPool {
public:
    explicit ThreadPool(size_t n = 4)  // limit to 4 workers
        : stop_(false) {
        if (n == 0) n = 1;
        if (n > 4) n = 4; // enforce cap for consistency

        workers_.reserve(n);
        for (size_t i = 0; i < n; ++i) {
            // spawn each worker thread
            workers_.emplace_back([this] {
                for (;;) {
                    std::function<void()> job;
                    {
                        std::unique_lock<std::mutex> lk(mx_);
                        cv_.wait(lk, [this] { return stop_ || !q_.empty(); });
                        if (stop_ && q_.empty()) return;
                        job = std::move(q_.front());
                        q_.pop();
                    }
                    job(); // execute the queued task
                }
                });
        }
    }

    ~ThreadPool() { shutdown(); }

    // Adds a job to the queue
    void enqueue(std::function<void()> f) {
        {
            std::lock_guard<std::mutex> lk(mx_);
            q_.push(std::move(f));
        }
        cv_.notify_one();
    }

    // Graceful shutdown — joins all threads
    void shutdown() {
        bool expected = false;
        if (!stop_.compare_exchange_strong(expected, true)) return;
        cv_.notify_all();
        for (auto& t : workers_)
            if (t.joinable()) t.join();
    }

private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> q_;
    std::mutex mx_;
    std::condition_variable cv_;
    std::atomic<bool> stop_;
};
