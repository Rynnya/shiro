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

#ifndef SHIRO_USER_HH
#define SHIRO_USER_HH

#include <chrono>

#include "../config/bot_file.hh"
#include "../io/layouts/user/user_presence.hh"
#include "../io/layouts/user/user_stats.hh"
#include "../io/layouts/user/user_status.hh"
#include "../io/queue.hh"
#include "../utils/play_mode.hh"
#include "../shiro.hh"

namespace shiro::users {

    class user_preferences {
    public:
        bool display_classic = false;
        bool display_relax = false;
        int8_t auto_classic = 0;
        int8_t auto_relax = 0;
        bool score_ow_std = false;
        bool score_ow_taiko = false;
        bool score_ow_ctb = false;
        bool score_ow_mania = false;

        user_preferences() = default;
        user_preferences(int32_t id);
        bool is_overwrite(shiro::utils::play_mode mode);
    };

    class user {
    public:
        int32_t user_id = 0;
        std::string password = "";
        std::string salt = "";
        std::string country = "XX";

        int64_t permissions = 0;
        std::vector<int32_t> friends {};

        // Session info
        std::string token = ""; // UUID v4
        std::string hwid = ""; // SHA256
        std::chrono::seconds last_ping = 0s;
        bool hidden = false; // If the client is hidden, e.g. restricted
        bool is_relax = false; // If user currently playing with relax or his last score was with relax

        std::string client_version = ""; // Full client version string sent on login (b20160403.6)
        int32_t client_build = 20131216; // Fixed client build version (20160403)
        uint32_t client_type = 0;

        io::layouts::user_presence presence;
        io::layouts::user_stats stats;
        io::layouts::user_status status;

        user_preferences preferences;

        io::queue queue;

        explicit user(int32_t user_id);
        explicit user(const std::string &username);

        bool init();
        void update(bool is_relax);
        void save_stats(bool to_relax);
        void update_country(const std::string& country);
        void update_counts(const std::string& rank);

        std::string get_url();
        std::string get_avatar_url();

        // Re-sends presence and stats to the user to refresh on client-side
        void refresh_stats();

        bool check_password(const std::string &password);
        void verify_address(const std::string& address);

    };

}

#endif //SHIRO_USER_HH
