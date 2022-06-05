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

#include "achievement.hh"

#include "impl/skill/combo.hh"
#include "impl/skill/full_combo.hh"
#include "impl/skill/mods.hh"
#include "impl/skill/pass.hh"
#include "impl/skill/playcount.hh"

#include "impl/bunny.hh"
#include "impl/maria.hh"

namespace storage {

    const std::array<std::unique_ptr<shiro::achievements::achievement>, 7> achievements = {
        std::make_unique<shiro::achievements::combo>(),
        std::make_unique<shiro::achievements::full_combo>(),
        std::make_unique<shiro::achievements::mods>(),
        std::make_unique<shiro::achievements::pass>(),
        std::make_unique<shiro::achievements::playcount>(),

        std::make_unique<shiro::achievements::bunny>(),
        std::make_unique<shiro::achievements::maria>(),
    };

}

std::string shiro::achievements::build(
    const std::shared_ptr<shiro::users::user>& user,
    const shiro::beatmaps::beatmap& beatmap,
    const shiro::scores::score& score
) {
    std::vector<std::pair<size_t, std::string>> achievements {};

    for (const auto& ach : storage::achievements) {
        ach->verify(achievements, user, beatmap, score);
    }

    std::string output {};
    for (const auto [id, ach_str] : achievements) {
        const auto it = std::find_if(
            user->stats.acquired_achievements.begin(), 
            user->stats.acquired_achievements.end(), 
            [id_ = id](std::pair<int64_t, bool> pair) { return pair.first == id_; }
        );
        if (it != user->stats.acquired_achievements.end()) {
            continue;
        }

        output += (ach_str + '/');
        user->stats.acquired_achievements.emplace_back(id, false);
    }

    if (!output.empty()) {
        output.pop_back();
    }

    return output;
}