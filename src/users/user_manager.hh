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

#ifndef SHIRO_USER_MANAGER_HH
#define SHIRO_USER_MANAGER_HH

#include <memory>
#include <shared_mutex>
#include <unordered_set>
#include <vector>

#include "user.hh"

namespace shiro::users::manager {

    extern std::vector<std::shared_ptr<user>> online_users;
    extern std::shared_timed_mutex mutex;

    extern std::unordered_set<int32_t> preferences_query;
    extern std::shared_timed_mutex query_mutex;

    // Adds a user to online users
    void login_user(const std::shared_ptr<user> &user);

    // Removes a user from online users
    void logout_user(const std::shared_ptr<user> &user);
    void logout_user(int32_t user_id);

    bool is_online(const std::shared_ptr<user> &user);
    bool is_online(int32_t user_id);
    bool is_online(const std::string &token);

    std::shared_ptr<user> get_user_by_username(const std::string &username);
    std::shared_ptr<user> get_user_by_id(int32_t id);
    std::shared_ptr<user> get_user_by_token(const std::string &token);

    // Works for both online and offline users
    std::string get_username_by_id(int32_t id);
    int32_t get_id_by_username(const std::string &username);

    void update_preferences(int32_t id);

    // Helper methods to quickly iterate over all online users
    void iterate(const std::function<void(std::shared_ptr<user>&)> &callback, bool skip_bot = false);
    void iterate(const std::function<void(size_t, std::shared_ptr<user>&)> &callback, bool skip_bot = false);

    size_t get_online_users();

}

#endif //SHIRO_USER_MANAGER_HH
