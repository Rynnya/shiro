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
#include "hits.hh"

#include "../../../utils/play_mode.hh"

// This value must be added to ID's
constexpr size_t achievement_offset = 20;

namespace keys {

    // Placeholders for 4 achievement (with more than 10 mil hits each...)

    const std::array<int32_t, 12> threshold = {
        30000, 300000, 3000000, 0,
        20000, 200000, 2000000, 0,
        40000, 400000, 4000000, 0
    };

    const std::array<std::string, 12> achievement = {
        "taiko-hits-30000", "taiko-hits-300000", "taiko-hits-300000", "",
        "fruits-hits-20000", "fruits-hits-200000", "fruits-hits-200000", "",
        "mania-hits-40000", "mania-hits-400000", "mania-hits-400000", ""
    };

    const std::array<std::string, 12> names = {
        "30,000 Drum Hits", "300,000 Drum Hits", "3,000,000 Drum Hits", "",
        "Catch 20,000 Fruits", "Catch 200,000 Fruits", "Catch 2,000,000 Fruits", "",
        "40,000 Keys", "400,000 Keys", "4,000,000 Keys", "",
    };

    const std::array<std::string, 12> descriptions = {
        // taiko
        "Did that drum have a face?",
        "The rhythm never stops.",
        "Truly, the Don of dons.",
        "",
        // ctb
        "That is a lot of dietary fiber.",
        "So, I heard you like fruit...",
        "Downright healthy.",
        "",
        // mania
        "Just the start of the rainbow.",
        "Four hundred thousand and still not even close.",
        "Is this the end of the rainbow?",
        ""
    };
}


void shiro::achievements::hits::verify(
    std::vector<std::pair<int64_t, std::string>>& output,
    const std::shared_ptr<shiro::users::user>& user,
    const shiro::beatmaps::beatmap& beatmap,
    const shiro::scores::score& score
) {
    const uint8_t play_mode = score.play_mode;
    const int32_t total_hits = user->stats.total_hits;

    if (play_mode == 0 || play_mode > 3 || play_mode != user->stats.play_mode) {
        return;
    }

    const int32_t offset = 4 * (play_mode - 1);

    for (size_t current = offset; current < offset + 3; current++) {
        const int32_t& required = keys::threshold[current];
        if (user->stats.total_hits >= required) {
            output.emplace_back(
                achievement_offset + offset + current,
                fmt::format("{}+{}+{}", keys::achievement[offset + current], keys::names[offset + current], keys::descriptions[offset + current])
            );
        }
    }
}
