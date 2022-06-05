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
#include "../../../utils/mods.hh"
#include "mods.hh"

// This value must be added to ID's
constexpr size_t achievement_offset = 112;

namespace keys {

    const std::array<std::string, 11> names = {
        "Finality",
        "Perfectionist",
        "Rock Around The Clock",
        "Time And A Half",
        "Sweet Rave Party",
        "Blindsight",
        "Are You Afraid Of The Dark?",
        "Dial It Right Back",
        "Risk Averse",
        "Slowboat",
        "Burned Out"
    };

    const std::array<std::string, 11> descriptions = {
        "High stakes, no regrets.",
        "Accept nothing but the best.",
        "You can't stop the rock.",
        "Having a right ol' time. One and a half of them, almost.",
        "Founded in the fine tradition of changing things that were just fine as they were.",
        "I can see just perfectly.",
        "Harder than it looks, probably because it's hard to look.",
        "Sometimes you just want to take it easy.",
        "Safety nets are fun!",
        "You got there. Eventually.",
        "One cannot always spin to win."
    };

    const std::array<std::string, 11> mod_name = {
        "suddendeath",
        "perfect",
        "hardrock",
        "doubletime",
        "nightcore",
        "hidden",
        "flashlight",
        "easy",
        "nofail",
        "halftime",
        "spunout"
    };

    const std::array<shiro::utils::mods, 11> available_mods = {
        shiro::utils::mods::sudden_death,
        shiro::utils::mods::perfect,
        shiro::utils::mods::hard_rock,
        shiro::utils::mods::double_time,
        shiro::utils::mods::nightcore,
        shiro::utils::mods::hidden,
        shiro::utils::mods::flashlight,
        shiro::utils::mods::easy,
        shiro::utils::mods::no_fail,
        shiro::utils::mods::half_time,
        shiro::utils::mods::spun_out
    };
}

void shiro::achievements::mods::verify(
    std::vector<std::pair<int64_t, std::string>>& output,
    const std::shared_ptr<shiro::users::user>& user, 
    const shiro::beatmaps::beatmap& beatmap, 
    const shiro::scores::score& score
) {
    const uint32_t mods = score.mods;

    if (mods == 0) {
        return;
    }

    for (size_t i = 0; i < keys::available_mods.size(); i++) {
        if (mods & keys::available_mods[i]) {
            output.emplace_back(achievement_offset + i, fmt::format("all-intro-{}+{}+{}", keys::mod_name[i], keys::names[i], keys::descriptions[i]));
        }
    }
}