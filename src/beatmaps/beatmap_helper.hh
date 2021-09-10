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

#ifndef SHIRO_BEATMAP_HELPER_HH
#define SHIRO_BEATMAP_HELPER_HH

#include <cstdio>
#include <optional>
#include <string>

#include "beatmap.hh"
#include "beatmap_ranked_status.hh"

namespace shiro::beatmaps::helper {

    void init();

    int32_t fix_beatmap_status(int32_t status_code);

    bool has_leaderboard(int32_t status_code);

    bool awards_pp(int32_t status_code);

    std::optional<std::string> get_location(int32_t beatmap_id, bool download = true);

    float score_to_difficulty(beatmaps::beatmap beatmap, utils::play_mode mode = utils::play_mode::standard);
    std::string build_difficulty_header(beatmaps::beatmap beatmap, utils::play_mode mode = utils::play_mode::standard);

}

#endif //SHIRO_BEATMAP_HELPER_HH
