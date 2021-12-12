/*
 * shiro - High performance, high quality osu!Bancho C++ re-implementation
 * Copyright (C) 2021 Rynnya
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "thread_pool.hh"

#ifndef SHIRO_WORKERS_AMOUNT
    #define SHIRO_WORKERS_AMOUNT (std::max)(std::thread::hardware_concurrency(), 8U)
#endif

shiro::thread::pool shiro::thread::curl_operations = shiro::thread::pool(SHIRO_WORKERS_AMOUNT);

shiro::thread::pool::pool(uint32_t threads) {
    for (uint32_t i = 0; i < threads; i++) {
        workers.emplace_back([this]() {
            while (true) {
                std::function<void()> task;

                {
                    std::unique_lock<std::mutex> lock(queue_lock);
                    condition.wait(lock, [this]() {
                        return !running || !tasks.empty();
                    });

                    if (!running) {
                        return;
                    }

                    if (tasks.empty()) {
                        continue;
                    }

                    task = std::move(tasks.front());
                    tasks.pop_front();
                }

                if (task) {
                    task();
                }
            }
        });
    }
}

shiro::thread::pool::~pool() {
    running = false;

    condition.notify_all();
    for (std::thread& th : workers) {
        th.join();
    }
}

void shiro::thread::pool::push_and_notify(std::function<void()>&& function) {
    {
        std::unique_lock<std::mutex> lock(queue_lock);
        tasks.push_back(std::move(function));
    }

    condition.notify_one();
}