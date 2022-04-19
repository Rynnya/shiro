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

#ifndef SHIRO_SCORE_HH
#define SHIRO_SCORE_HH

#include <cstdint>
#include <string>
#include <vector>

#include "../utils/play_mode.hh"

namespace shiro::scores {

    class score {
    public:
        score() = default;

        explicit score(int32_t id) : id(id) {
            // Initialized in initializer list
        }

        int32_t id = 0;
        int32_t user_id = 0;
        std::string hash = "";
        std::string beatmap_md5 = "";

        std::string rank = "F";
        int64_t total_score = 0;
        int32_t max_combo = 0;
        float pp = 0.0f;

        float accuracy = 0.0f;
        int64_t mods = 0;

        bool fc = false;
        bool passed = false;
        bool is_relax = false;

        int32_t count_300 = 0;
        int32_t count_100 = 0;
        int32_t count_50 = 0;
        int32_t count_katus = 0;
        int32_t count_gekis = 0;
        int32_t count_misses = 0;

        uint8_t play_mode = static_cast<uint8_t>(utils::play_mode::standard);
        int64_t time = 0;
        int32_t play_time = 0;

        std::string to_string(std::vector<score> &scores, bool is_relax);

    };

}

#endif //SHIRO_SCORE_HH
