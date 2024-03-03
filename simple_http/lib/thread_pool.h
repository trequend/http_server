// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#pragma once

#include <cassert>
#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace simple_http {

template <typename ThreadState>
class ThreadPool {
   public:
    typedef void Task(ThreadState* state);

    typedef std::unique_ptr<ThreadState> ThreadStateFactory(size_t index);

    ~ThreadPool() {
        {
            std::unique_lock lock(mutex_);
            stopped_ = true;
        }
        condition_.notify_all();
        for (ThreadData& data : threads_data_) {
            data.thread.join();
        }
    }

    static std::unique_ptr<ThreadPool<ThreadState>> create(
        size_t threads_count,
        const std::function<ThreadStateFactory>& create_state) {
        assert(threads_count >= 0);

        if (threads_count == 0) {
            return nullptr;
        }

        std::unique_ptr<ThreadPool<ThreadState>> pool(new ThreadPool());
        if (pool->initialize(threads_count, create_state)) {
            return pool;
        } else {
            return nullptr;
        }
    }

    void post(std::function<Task>&& task) {
        {
            std::unique_lock lock(mutex_);
            tasks_.push(std::move(task));
        }
        condition_.notify_one();
    }

    size_t getPlannedTasksCount() {
        std::unique_lock lock(mutex_);
        return tasks_.size();
    }

   private:
    struct ThreadData {
        std::thread thread;
        std::unique_ptr<ThreadState> state;
    };

    ThreadPool() = default;

    bool initialize(size_t threads_count,
                    const std::function<ThreadStateFactory>& create_state) {
        try {
            for (size_t i = 0; i < threads_count; i++) {
                ThreadData data;

                data.state = std::move(create_state(i));
                if (data.state == nullptr) {
                    return false;
                }

                std::thread thread([this, i] {
                    while (true) {
                        std::function<Task> task;

                        {
                            std::unique_lock lock(mutex_);
                            condition_.wait(lock, [this] {
                                return stopped_ || !tasks_.empty();
                            });
                            if (stopped_) {
                                return;
                            }

                            task = std::move(tasks_.front());
                            tasks_.pop();
                        }

                        try {
                            task(threads_data_[i].state.get());
                        } catch (...) {
                            continue;
                        }
                    }
                });
                data.thread = std::move(thread);

                threads_data_.emplace_back(std::move(data));
            }
        } catch (...) {
            return false;
        }

        return true;
    }

    std::vector<ThreadData> threads_data_;

    std::mutex mutex_;
    std::condition_variable condition_;
    std::queue<std::function<Task>> tasks_;
    bool stopped_ = false;
};

}  // namespace simple_http
