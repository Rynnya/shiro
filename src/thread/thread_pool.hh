/*
 * shiro - High performance, high quality osu!Bancho C++ re-implementation
 * Copyright (C) 2021-2022 Rynnya
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

#ifndef SHIRO_THREAD_POOL_HH
#define SHIRO_THREAD_POOL_HH

#include <cstdint>
#include <deque>
#include <functional>
#include <future>
#include <vector>

namespace shiro::thread {

    class pool {
    public:
        pool() = delete;
        pool(uint32_t threads);

        ~pool();

        pool(const pool& other) = delete;
        pool(pool&& other) = delete;

        pool& operator=(const pool& other) = delete;
        pool& operator=(pool&& other) = delete;

        // Executes the function and returns std::future<func_type>
        template <typename F, typename... Args>
        auto push(F&& func, Args&&... args) -> std::future<decltype(func(args...))> {
            std::function<decltype(func(args...))()> bounded_function = std::bind(std::forward<F>(func), std::forward<Args>(args)...);
            auto task_ptr = std::make_shared<std::packaged_task<decltype(func(args...))()>>(bounded_function);
            std::function<void()> wrapper_func = [task_ptr]() { (*task_ptr)(); };

            push_and_notify(std::move(wrapper_func));
            return task_ptr->get_future();
        }

        // Absorbs a function and executes it asynchronously
        template <typename F, typename... Args>
        void push_and_forgot(F&& func, Args&&... args) {
            std::function<decltype(func(args...))()> bounded_function = std::bind(std::forward<F>(func), std::forward<Args>(args)...);
            std::function<void()> wrapper = [bounded_function]() { bounded_function(); };

            push_and_notify(std::move(wrapper));
        }

    private:
        void push_and_notify(std::function<void()>&& function);

        std::mutex queue_lock;
        std::condition_variable condition;
        std::atomic_bool running = true;

        std::vector<std::thread> workers;
        std::deque<std::function<void()>> tasks;
    };

    extern pool event_loop;
}

#endif
