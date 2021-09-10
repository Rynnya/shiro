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

#include <algorithm>
#include <chrono>

#include "mods.hh"
#include "string_utils.hh"
#include "time_utils.hh"

int64_t shiro::utils::time::unix_epoch_ticks = 621355968000000000;

std::unordered_map<std::string, uint32_t> shiro::utils::time::duration_mapping = {
    { "s", 1 },
    { "min", 60 },
    { "h", 60 * 60 },
    { "d", 60 * 60 * 24 },
    { "w", 60 * 60 * 24 * 7 },
    { "m", 60 * 60 * 24 * 7 * 30 },
    //{ "y", 60 * 60 * 24 * 7 * 30 * 365 }
};

int64_t shiro::utils::time::get_current_time_ticks() {
    using ticks = std::chrono::duration<int64_t, std::ratio_multiply<std::ratio<100>, std::nano>>;

    std::chrono::time_point now = std::chrono::system_clock::now();
    int64_t ticks_since_epoch = std::chrono::duration_cast<ticks>(now.time_since_epoch()).count();
    ticks_since_epoch += unix_epoch_ticks;

    return ticks_since_epoch;
}

std::optional<uint32_t> shiro::utils::time::parse_time_string(const std::string &input)
{
    if (input.empty())
        return std::nullopt;

    int64_t it = 0;
    int64_t size = input.size();
    std::vector<char> chars;

    std::string raw_unit = "s";
    std::string raw_time = input;
    uint32_t time = 0;

    // Only parse the time suffix if we actually have one
    for (char last_char = input[size - ++it]; std::isalpha(last_char);)
    {
        chars.emplace_back(last_char);
        last_char = input[size - ++it];
    }

    // If non is alpha - don't touch anything
    if (chars.size() != 0)
    {
        std::reverse(chars.begin(), chars.end());
        raw_unit = std::string(chars.begin(), chars.end());
        raw_time = input.substr(0, size - it);
    }

    if (!strings::safe_uint(raw_time, time))
        return std::nullopt;

    auto iterator = duration_mapping.find(raw_unit);

    if (iterator == duration_mapping.end())
        return std::nullopt;

    return time * iterator->second;
}

int32_t shiro::utils::time::adjusted_seconds(int32_t mods, int32_t time)
{
    if (mods & static_cast<int32_t>(shiro::utils::mods::double_time))
        return time / 1.5;

    if (mods & static_cast<int32_t>(shiro::utils::mods::half_time))
        return time / 0.75;

    return time;
}
