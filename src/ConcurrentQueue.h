#pragma once
#include <queue>
#include <mutex>
#include <optional>

/*
 * Coms for worker threads to pass decoded images/assets to the main thread
 * follows fifo manner
 */
template <typename T>
class ConcurrentQueue {
public:
    void push(T v) {
        std::lock_guard<std::mutex> lk(mx_);
        q_.push(std::move(v));
    }

    std::optional<T> try_pop() {
        std::lock_guard<std::mutex> lk(mx_);
        if (q_.empty()) return std::nullopt;
        T v = std::move(q_.front());
        q_.pop();
        return v;
    }

    size_t size() const {
        std::lock_guard<std::mutex> lk(mx_);
        return q_.size();
    }

private:
    mutable std::mutex mx_;
    std::queue<T> q_;
};
