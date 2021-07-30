/*
 * shiro - High performance, high quality osu!Bancho C++ re-implementation
 * Copyright (C) 2018-2020 Marc3842h, czapek
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

#include "../thirdparty/loguru.hh"
#include "redis.hh"
#include "../users/user_manager.hh"
#include "../utils/string_utils.hh"

shiro::redis::redis(std::string address, uint32_t port, std::string password)
    : address(std::move(address))
    , port(port)
    , password(std::move(password)){
    // Initialized in initializer list
}

void shiro::redis::connect() {
    if (is_connected())
        return;

    this->client = std::make_shared<cpp_redis::client>();
    this->sub = std::make_shared<cpp_redis::subscriber>();
    this->client->connect(this->address, this->port);
    this->sub->connect(this->address, this->port);

    if (!this->password.empty())
    {
        this->client->auth(this->password);
        this->sub->auth(this->password);
    }

    if (!is_connected())
        ABORT_F("Unable to connect to Redis instance.");

    LOG_F(INFO, "Successfully connected to Redis.");
}

void shiro::redis::disconnect() {
    std::chrono::seconds seconds = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
    );
    this->client->set("shiro.startup", "0");
    this->client->set("shiro.shutdown", std::to_string(seconds.count()));

    this->client->sync_commit();

    if (!is_connected())
        return;

    this->client->disconnect();
    this->sub->disconnect();

    LOG_F(INFO, "Successfully disconnected from Redis.");
}

void shiro::redis::setup() {
    this->client->set("shiro.online_users", "0");

    std::chrono::seconds seconds = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
    );
    this->client->set("shiro.startup", std::to_string(seconds.count()));
    this->client->set("shiro.shutdown", "0");

    this->client->sync_commit();

    this->sub->subscribe("shiro.user_preferences", 
        [](const std::string& ch, const std::string& msg) { users::manager::update_preferences(utils::strings::safe_int(msg)); });
    this->sub->commit();
}

bool shiro::redis::is_connected() {
    return this->client != nullptr;
}

std::shared_ptr<cpp_redis::client> shiro::redis::get() {
    if (!is_connected())
        ABORT_F("Tried to acquire redis connection while not being connected.");

    return this->client;
}
