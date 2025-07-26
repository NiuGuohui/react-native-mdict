#include "CancellableThreadPool.h"

namespace JSMdict {
    CancellableThreadPool::TaskID CancellableThreadPool::enqueue(std::function<void()> task) {
        std::lock_guard<std::mutex> lock(queueMutex_);
        TaskID id = ++nextTaskId_;
        tasks_.emplace(id, std::move(task));
        taskQueue_.push(id);
        cv_.notify_one();
        return id;
    }

    void CancellableThreadPool::cancel(TaskID id) {
        if (id) {
            std::lock_guard<std::mutex> lock(queueMutex_);
            cancelled_.insert(id);
        }
    }

    void CancellableThreadPool::workerLoop() {
        while (true) {
            TaskID taskId;
            std::function<void()> task;

            {
                std::unique_lock<std::mutex> lock(queueMutex_);
                cv_.wait(lock, [this]() { return stop_ || !taskQueue_.empty(); });

                if (stop_ && taskQueue_.empty()) return;

                taskId = taskQueue_.front();
                taskQueue_.pop();

                if (cancelled_.contains(taskId)) {
                    cancelled_.erase(taskId);
                    tasks_.erase(taskId);
                    continue;
                }

                if (!tasks_.contains(taskId)) {
                    tasks_.erase(taskId);
                    continue;
                }

                task = std::move(tasks_[taskId]);
            }

            task();
        }
    }


    void CancellableThreadPool::stop() {
        {
            std::lock_guard<std::mutex> lock(queueMutex_);
            stop_ = true;
        }
        cv_.notify_all();
    }
}