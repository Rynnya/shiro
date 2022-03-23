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

#ifndef SHIRO_DATABASE_HH
#define SHIRO_DATABASE_HH

#include <sqlpp11/mysql/mysql.h>
#include <sqlpp11/sqlpp11.h>

#include <deque>
#include <memory>
#include <mutex>
#include <string>

namespace shiro::database {

    class pool;

    extern std::unique_ptr<pool> instance;

    class connection {
    public:
        friend class pool;
        typedef std::unique_ptr<sqlpp::mysql::connection> handle_t;

        ~connection();
        
        connection() = delete;
        connection(const connection& other) = delete;
        connection& operator=(const connection& other) = delete;

        connection(connection&& other) noexcept;
        connection& operator=(connection&& other) noexcept;

        sqlpp::mysql::connection& operator*() const noexcept;

        template <typename T>
        auto operator()(const T& sql) const {
            return this->handle->operator()(sql);
        }

    private:
        connection(const std::shared_ptr<sqlpp::mysql::connection_config>& config);
        connection(handle_t&& handle);

        void dispose() noexcept;

        handle_t handle = nullptr;
    };

    class pool {
    public:
        friend class connection;

        pool(const std::string& address, uint32_t port, const std::string& db_name, const std::string& username, const std::string& password, size_t size = 10);

        connection pop();
        void destroy();

    private:
        void push(connection::handle_t&& handle);
        connection create_new();

        std::shared_ptr<sqlpp::mysql::connection_config> config = nullptr;
        std::deque<connection> stack = {};
        std::mutex mtx;
    };
}

#endif //SHIRO_DATABASE_HH
