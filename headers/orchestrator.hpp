#ifndef ORCHESTRATOR
#define ORCHESTRATOR

#include <atomic>
#include <condition_variable>
#include <cstdio>
#include <iostream>
#include <filesystem>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <string>
#include <syncstream>
#include <thread>

namespace {
    const unsigned int CORES{ std::thread::hardware_concurrency() };

    template<typename T, typename Callable>
    void run_with_thread_pool(std::vector<T>& items, Callable&& lambda) {
        std::queue<T*> task_queue;

        for (auto& item: items) {
            task_queue.push(&item);
        }
        
        std::mutex queue_mutex;
        std::vector<std::thread> pool;

        for (unsigned int i = 0; i < CORES; ++i) {
            pool.emplace_back([&] {
                while (true) {
                    T *item{ nullptr };
                    {
                        std::lock_guard<std::mutex> lock(queue_mutex);

                        if (task_queue.empty()) {
                            return;
                        }

                        item = task_queue.front();

                        task_queue.pop();
                    }

                    lambda(*item);
                }
            });
        }

        for (auto& thread: pool) {
            thread.join();
        }
    }

    template<typename T, typename Callable>
    void run_with_futures(std::vector<T>& items, Callable&& lambda) {
        std::vector<std::future<void>> futures;

        for (auto& item: items) {
            futures.emplace_back(std::async(std::launch::async, lambda, item));
        }

        for (auto& future: futures) {
            future.get();
        }
    }
}

namespace orchestrator {
    template<typename T, typename Callable>
    void orchestrate_task(std::vector<T>& items, Callable&& lambda) {
        if (items.size() >= CORES) {
            run_with_thread_pool(items, std::forward<Callable>(lambda));
        } else {
            run_with_futures(items, std::forward<Callable>(lambda));
        }
    }
}

#endif
