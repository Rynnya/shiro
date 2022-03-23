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

#include "../thirdparty/naga.hh"
#include "database.hh"

std::unique_ptr<shiro::database::pool> shiro::database::instance;

shiro::database::connection::~connection() {
    if (handle) {
        shiro::database::instance->push(std::move(handle));
    }
}

shiro::database::connection::connection(connection&& other) noexcept {
    this->handle = std::move(other.handle);
}

shiro::database::connection& shiro::database::connection::operator=(connection&& other) noexcept {
    this->handle = std::move(other.handle);
    return *this;
}

sqlpp::mysql::connection& shiro::database::connection::operator*() const noexcept {
    return *handle;
}

shiro::database::connection::connection(const std::shared_ptr<sqlpp::mysql::connection_config>& config) {
    this->handle = std::make_unique<sqlpp::mysql::connection>(config);
}

shiro::database::connection::connection(handle_t&& handle) {
    this->handle = std::move(handle);
}

void shiro::database::connection::dispose() noexcept {
    handle.reset(nullptr);
}

shiro::database::pool::pool(const std::string& address, uint32_t port, const std::string& db_name, const std::string& username, const std::string& password, size_t size) {
    this->config = std::make_shared<sqlpp::mysql::connection_config>();

    this->config->host = address;
    this->config->port = port;
    this->config->database = db_name;
    this->config->user = username;
    this->config->password = password;
    this->config->auto_reconnect = true;

    #if defined(_DEBUG) && !defined(SHIRO_NO_SQL_DEBUG)
        this->config->debug = true;
    #endif

    for (size_t i = 0; i < size; i++) {
        stack.push_back(create_new());
    }

    LOG_F(INFO, "Database pool was initialized with initial size of {}", size);
}

shiro::database::connection shiro::database::pool::pop() {
    std::unique_lock<std::mutex> lock(mtx);

    if (stack.empty()) {
        lock.unlock();
        return create_new();
    }

    shiro::database::connection ptr = std::move(stack.front());
    stack.pop_front();

    return ptr;
}

void shiro::database::pool::destroy() {
    // Don't nullify config, as this might cause crash on exit
    for (auto& db : stack) {
        // ~connection() will gradually close connection to database
        db.dispose();
    }
}

void shiro::database::pool::push(shiro::database::connection::handle_t&& handle) {
    std::unique_lock<std::mutex> lock(mtx);
    stack.push_back(shiro::database::connection{ std::move(handle) });
}

shiro::database::connection shiro::database::pool::create_new() {
    if (config == nullptr) {
        return nullptr;
    }

    return shiro::database::connection{ config };
}