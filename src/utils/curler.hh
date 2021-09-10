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

#ifndef SHIRO_CURLER_HH
#define SHIRO_CURLER_HH

#include <tuple>
#include <string>

#include "../thirdparty/json.hh"

namespace shiro::utils::curl {

    // Returns a boolean indicating if the request was successful and the response as a string if it was successful.
    std::tuple<bool, std::string> get(const std::string &url);

    // Same as get(url) but with support for direct modes (Emulation, Beatconnect, Cheesegull)
    std::tuple<bool, std::string> get_direct(const std::string &url);

    // Used for Discord Webhook
    bool post_message(const std::string &url, const nlohmann::json &message);

    std::string escape_url(const std::string &raw);
    std::string unescape_url(const std::string &raw);

    size_t internal_callback(void *raw_data, size_t size, size_t memory, std::string *ptr);

}

#endif //SHIRO_CURLER_HH
