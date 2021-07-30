/*
 * shiro - High performance, high quality osu!Bancho C++ re-implementation
 * Copyright (C) 2018-2020 Marc3842h, czapek
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

#include <algorithm>

#include "../database/tables/user_table.hh"
#include "../thirdparty/loguru.hh"
#include "../utils/osu_client.hh"
#include "user_manager.hh"

std::vector<std::shared_ptr<shiro::users::user>> shiro::users::manager::online_users;
std::shared_timed_mutex shiro::users::manager::mutex;

std::unordered_set<int32_t> shiro::users::manager::preferences_query;
std::shared_timed_mutex shiro::users::manager::query_mutex;

void shiro::users::manager::login_user(std::shared_ptr<shiro::users::user> user)
{
    if (user == nullptr || user->token.empty())
        return;

    if (is_online(user))
    {
        logout_user(user);
        return;
    }

    // Disallow other threads from both writing and reading
    std::unique_lock<std::shared_timed_mutex> lock(mutex);

    online_users.emplace_back(user);

    LOG_F(INFO, "User %s logged in successfully.", user->presence.username.c_str());

    if (user->client_type != +utils::clients::osu_client::aschente && !user->hidden)
        redis_connection->get()->incr("shiro.online_users", nullptr).commit();
}

void shiro::users::manager::logout_user(std::shared_ptr<shiro::users::user> user)
{
    if (user == nullptr || !is_online(user))
        return;

    // Disallow other threads from both writing and reading
    std::unique_lock<std::shared_timed_mutex> lock(mutex);

    auto iterator = std::find(online_users.begin(), online_users.end(), user);

    if (iterator == online_users.end())
        return;

    online_users.erase(iterator);

    LOG_F(INFO, "User %s logged out successfully.", user->presence.username.c_str());

    if (user->client_type != +utils::clients::osu_client::aschente && !user->hidden)
        redis_connection->get()->decr("shiro.online_users", nullptr).commit();
}

void shiro::users::manager::logout_user(int32_t user_id) 
{
    std::shared_ptr<user> target_user = get_user_by_id(user_id);

    if (target_user == nullptr)
        return;

    logout_user(target_user);
}

bool shiro::users::manager::is_online(std::shared_ptr<shiro::users::user> user)
{
    if (user == nullptr)
        return false;

    // Disallow other threads from writing (but not from reading)
    std::shared_lock<std::shared_timed_mutex> lock(mutex);

    return std::find(online_users.begin(), online_users.end(), user) != online_users.end();
}

bool shiro::users::manager::is_online(int32_t user_id)
{
    if (user_id < 10)
        return false;

    // Disallow other threads from writing (but not from reading)
    std::shared_lock<std::shared_timed_mutex> lock(mutex);

    return std::any_of(online_users.begin(), online_users.end(),
        [user_id](const std::shared_ptr<user>& user) { return user->user_id == user_id; });
}

bool shiro::users::manager::is_online(const std::string &token) {
    if (token.empty())
        return false;

    // Disallow other threads from writing (but not from reading)
    std::shared_lock<std::shared_timed_mutex> lock(mutex);

    return std::any_of(online_users.begin(), online_users.end(),
        [token](const std::shared_ptr<user>& user) { return user->token == token; });
}

std::shared_ptr<shiro::users::user> shiro::users::manager::get_user_by_username(const std::string &username)
{
    if (username.empty())
        return nullptr;

    // Disallow other threads from writing (but not from reading)
    std::shared_lock<std::shared_timed_mutex> lock(mutex);

    for (const std::shared_ptr<user> &user : online_users)
        if (user->presence.username == username)
            return user;

    return nullptr;
}

std::shared_ptr<shiro::users::user> shiro::users::manager::get_user_by_id(int32_t id)
{
    // Disallow other threads from writing (but not from reading)
    std::shared_lock<std::shared_timed_mutex> lock(mutex);

    for (const std::shared_ptr<user> &user : online_users)
        if (user->user_id == id)
            return user;

    return nullptr;
}

std::shared_ptr<shiro::users::user> shiro::users::manager::get_user_by_token(const std::string &token)
{
    if (token.empty())
        return nullptr;

    // Disallow other threads from writing (but not from reading)
    std::shared_lock<std::shared_timed_mutex> lock(mutex);

    for (const std::shared_ptr<user> &user : online_users)
        if (user->token == token)
            return user;

    return nullptr;
}

std::string shiro::users::manager::get_username_by_id(int32_t id)
{
    std::shared_ptr<user> user = get_user_by_id(id);

    if (user != nullptr)
        return user->presence.username;

    sqlpp::mysql::connection db(db_connection->get_config());
    const tables::users user_table {};

    auto result = db(select(all_of(user_table)).from(user_table).where(user_table.id == id).limit(1u));

    if (result.empty())
        return "";

    return result.front().username;
}

int32_t shiro::users::manager::get_id_by_username(const std::string &username)
{
    std::shared_ptr<user> user = get_user_by_username(username);

    if (user != nullptr)
        return user->user_id;

    sqlpp::mysql::connection db(db_connection->get_config());
    const tables::users user_table {};

    auto result = db(select(all_of(user_table)).from(user_table).where(user_table.username == username).limit(1u));

    if (result.empty())
        return -1;

    return result.front().id;
}

void shiro::users::manager::update_preferences(int32_t id)
{
    // Disallow other threads from writing (but not from reading)
    std::shared_lock<std::shared_timed_mutex> lock(mutex);

    for (const std::shared_ptr<user>& user : online_users)
    {
        if (user->user_id != id)
            continue;

        user->preferences = std::move(shiro::users::user_preferences(id));
    }
}

void shiro::users::manager::iterate(const std::function<void(std::shared_ptr<shiro::users::user>)> &callback, bool skip_bot)
{
    // Disallow other threads from writing (but not from reading)
    std::shared_lock<std::shared_timed_mutex> lock(mutex);

    for (const std::shared_ptr<user> &user : online_users)
    {
        if (user->user_id == 1 && skip_bot)
            continue;

        callback(user);
    }
}

void shiro::users::manager::iterate(const std::function<void(size_t, std::shared_ptr<shiro::users::user>)> &callback, bool skip_bot)
{
    // Disallow other threads from writing (but not from reading)
    std::shared_lock<std::shared_timed_mutex> lock(mutex);

    for (size_t i = 0; i < online_users.size(); i++)
    {
        std::shared_ptr<user> user = online_users.at(i);

        if (user->user_id == 1 && skip_bot)
            continue;

        callback(i, user);
    }
}

size_t shiro::users::manager::get_online_users()
{
    // Disallow other threads from writing (but not from reading)
    std::shared_lock<std::shared_timed_mutex> lock(mutex);

    return std::count_if(online_users.begin(), online_users.end(),
        [](std::shared_ptr<users::user> &user) { return user->user_id != 1 && !user->hidden; });
}
