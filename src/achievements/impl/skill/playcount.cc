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

#include <array>

#include "../../../thirdparty/fmt/format.hh"
#include "playcount.hh"

 // This value must be added to ID's
constexpr size_t achievement_offset = 16;

namespace keys {

    const std::array<int32_t, 4> threshold = { 5000, 15000, 25000, 50000 };

    const std::array<std::string, 4> names = {
        "5,000 Plays",
        "15,000 Plays",
        "25,000 Plays",
        "50,000 Plays"
    };

    const std::array<std::string, 4> descriptions = {
        "There's a lot more where that came from.",
        "Must.. click.. circles..",
        "There's no going back.",
        "You're here forever."
    };

}

void shiro::achievements::playcount::verify(
    std::vector<std::pair<int64_t, std::string>>& output,
    const std::shared_ptr<shiro::users::user>& user, 
    const shiro::beatmaps::beatmap& beatmap, 
    const shiro::scores::score& score
) {
    const uint8_t play_mode = score.play_mode;
    if (play_mode != 0) {
        return;
    }

    for (size_t current = 0; current < keys::threshold.size(); current++) {
        const int32_t& required = keys::threshold[current];
        if (user->stats.play_count >= required) {
            output.emplace_back(
                achievement_offset + current,
                fmt::format("osu-plays-{}+{}+{}", required, keys::names[current], keys::descriptions[current])
            );
        }
    }
}
