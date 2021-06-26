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

#ifndef SHIRO_DISCORD_WEBHOOK_HH
#define SHIRO_DISCORD_WEBHOOK_HH

#include <string>

#include "../beatmaps/beatmap.hh"
#include "../thirdparty/json.hh"
#include "../users/user.hh"

namespace shiro::channels::discord_webhook
{
    enum class colors : uint32_t
    {
        OK = 64154,
        Blurple = 7506394,
        Error = 16711680,
        Ellysa = 16761034
    };

    void init();

    void send_message(const std::string &message);
    void send_message(const nlohmann::json &message);

    // Make own username and avatar (if override is true)
    nlohmann::json create_basis();
    // Helper to create simple message
    nlohmann::json create_embed(std::string title, std::string description = "", uint32_t color = 0);
    // Helper to get Discord emote from rank
    std::string get_rank_emote(std::string rank);
    
    // Regular helpers
    void send_restrict_message(std::string username, std::string origin_username, std::string reason);
    void send_ban_message(std::string username, std::string origin_username, std::string reason);
    void send_silence_message(std::string username, std::string origin_username, std::string reason, uint32_t duration);

    void send_top1_message(std::shared_ptr<shiro::users::user> user, shiro::beatmaps::beatmap beatmap, scores::score score);
}

#endif
