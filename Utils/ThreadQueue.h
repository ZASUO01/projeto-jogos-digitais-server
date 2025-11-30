//
// Created by pedro-souza on 28/11/2025.
//
#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

template <typename T>
class ThreadQueue {
private:
    std::queue<T> mQueue;
    std::mutex mMutex;
    std::condition_variable mCondition;

public:
    void Enqueue(T item) {
        {
            std::lock_guard lock(mMutex);
            mQueue.push(std::move(item));
        }
        mCondition.notify_one();
    }

    bool TryDequeue(T& item) {
        std::lock_guard lock(mMutex);
        if (mQueue.empty()) {
            return false;
        }
        item = std::move(mQueue.front());
        mQueue.pop();
        return true;
    }

    T WaitAndDequeue() {
        std::unique_lock lock(mMutex);

        mCondition.wait(lock, [this] { return !mQueue.empty(); });

        T item = std::move(mQueue.front());
        mQueue.pop();
        return item;
    }

    [[nodiscard]] bool empty() const {
        std::lock_guard lock(mMutex);
        return mQueue.empty();
    }
};