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
#include "combo.hh"

 // This value must be added to ID's
constexpr size_t achievement_offset = 0;

namespace keys {

    const std::array<int32_t, 4> threshold = { 500, 1000, 1500, 2000 };
    const std::array<std::string, 4> mode = { "std", "taiko", "ctb", "mania" };

}

void shiro::achievements::combo::verify(
    std::vector<std::pair<int64_t, std::string>>& output,
    const std::shared_ptr<shiro::users::user>& user, 
    const shiro::beatmaps::beatmap& beatmap, 
    const shiro::scores::score& score
) {
    const uint8_t play_mode = beatmap.play_mode;
    if (play_mode > 3) {
        return;
    }

    for (size_t i = 0; i < keys::threshold.size(); i++) {
        const int32_t& required = keys::threshold[i];
        if (score.max_combo > required) {
            output.emplace_back(
                achievement_offset + (play_mode * 4 + i),
                fmt::format(
                    "osu-combo-{}+{} Combo (osu!{})+{} big ones! You're moving up in the world!", 
                    required, required, keys::mode[play_mode], required
                )
            );
        }
    }
}
