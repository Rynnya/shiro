/*
 * shiro - High performance, high quality osu!Bancho C++ re-implementation
 * Copyright (C) 2018-2020 Marc3842h, czapek
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

#ifndef SHIRO_PERMISSIONS_HH
#define SHIRO_PERMISSIONS_HH

#include <cstdint>

namespace shiro::permissions {

    enum class perms : int64_t {
        // Basic permissions (Any new user will have those, can only be removed when restricted or banned)
        read = 1LL << 0,            // Always available (Even when banned)
        write = 1LL << 1,           // Some paths will be blocked when restricted or banned

        // API permissions, must be with prefix `api_`
        api_beatmap = 1LL << 16,

        // Access to console chat
        console_chat = 1LL << 17,

        // Cache control permissions, allows to clear specific cache
        cache_avatars = 1LL << 18,  // Can clear both single target and whole cache
        cache_mirror = 1LL << 19,   // Not used if mirror isn't yours
        cache_api = 1LL << 20,
        cache_shiro = 1LL << 21,

        // User manage permissions
        info_users = 1LL << 22,
        kick_users = 1LL << 23,
        silence_users = 1LL << 24,
        restrict_users = 1LL << 25,
        ban_users = 1LL << 26,

        manage_users = info_users | kick_users | silence_users | restrict_users | ban_users,

        // Shiro commands, must be with postfix `_cmd`
        announce_cmd = 1LL << 28,
        clear_cmd = 1LL << 29,
        rtx_cmd = 1LL << 30,
        recalculate_cmd = 1LL << 31,
        restart_cmd = 1LL << 32,

        admin = 
            read            |
            write           |
            api_beatmap     |
            console_chat    |
            cache_avatars   |
            cache_mirror    |
            cache_api       |
            cache_shiro     |
            manage_users    |
            announce_cmd    |
            clear_cmd       |
            rtx_cmd         |
            recalculate_cmd |
            restart_cmd
    };

    constexpr int64_t operator|(perms this_, perms other_) {
        return static_cast<int64_t>(this_) | static_cast<int64_t>(other_);
    }

    constexpr int64_t operator|(int64_t this_, perms other_) {
        return this_ | static_cast<int64_t>(other_);
    }

    constexpr int64_t operator&(int64_t this_, perms other_) {
        return this_ & static_cast<int64_t>(other_);
    }

    uint8_t get_chat_color(int64_t permissions);
}

#endif  // SHIRO_PERMISSIONS_HH
