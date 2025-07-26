#pragma once

#include <functional>
#include <future>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <atomic>
#include <optional>

namespace JSMdict {


    class CancellableThreadPool {
    public:
        using TaskID = uint64_t;

        CancellableThreadPool(size_t maxThreads)
                : maxThreads_(fmax(2, maxThreads)), nextTaskId_(1) {
            for (size_t i = 0; i < maxThreads_; ++i) {
                workers_.emplace_back([this] { this->workerLoop(); });
            }
        }

        ~CancellableThreadPool() {
            {
                std::lock_guard<std::mutex> lock(queueMutex_);
                stop_ = true;
                cv_.notify_all();
            }

            for (auto &t: workers_) {
                if (t.joinable()) {
                    t.join();
                }
            }
        }

        TaskID enqueue(std::function<void()> task);

        void cancel(TaskID id);

        void stop();

    private:
        void workerLoop();

    private:
        size_t maxThreads_;
        std::vector<std::thread> workers_;

        std::queue<TaskID> taskQueue_;
        std::unordered_map<TaskID, std::function<void()>> tasks_;
        std::unordered_set<TaskID> cancelled_;

        std::mutex queueMutex_;
        std::condition_variable cv_;
        std::atomic<TaskID> nextTaskId_;
        bool stop_ = false;
    };
}
