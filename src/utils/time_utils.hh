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

#ifndef SHIRO_TIME_UTILS_HH
#define SHIRO_TIME_UTILS_HH

#include <cstdint>
#include <chrono>
#include <optional>
#include <string>
#include <unordered_map>

namespace shiro::utils::time {

    extern int64_t unix_epoch_ticks;

    extern std::unordered_map<std::string, uint32_t> duration_mapping;

    int64_t get_current_time_ticks();

    template <typename T = std::chrono::seconds>
    inline T current_time()
    {
        return std::chrono::duration_cast<T>(std::chrono::system_clock::now().time_since_epoch());
    }

    // Returns seconds of the parsed time string, e.g input: "15min" returns 900 (seconds)
    std::optional<uint32_t> parse_time_string(const std::string &input);

    int32_t adjusted_seconds(int32_t mods, int32_t time);

}

#endif //SHIRO_TIME_UTILS_HH
