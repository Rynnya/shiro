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

#include "../../thirdparty/fmt/format.hh"
#include "bunny.hh"

// This value must be added to ID's
// Because Bunny is single achievement - offset will represent it's value
constexpr size_t achievement_offset = 123;

void shiro::achievements::bunny::verify(
    std::vector<std::pair<size_t, std::string>>& output, 
    const std::shared_ptr<shiro::users::user>& user, 
    const shiro::beatmaps::beatmap& beatmap, 
    const shiro::scores::score& score 
) {
    if (score.play_mode == 0 && beatmap.beatmapset_id == 184 && score.fc) {
        output.emplace_back(achievement_offset, "all-secret-bunny+Don't let the bunny distract you!+The order was indeed, not a rabbit.");
    }
}
