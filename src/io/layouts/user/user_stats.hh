/*
 * shiro - High performance, high quality osu!Bancho C++ re-implementation
 * Copyright (C) 2018-2020 Marc3842h, czapek
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

#ifndef SHIRO_USER_STATS_HH
#define SHIRO_USER_STATS_HH

#include "../serializable.hh"

namespace shiro::io::layouts {

    class user_stats : public serializable<void> {
    public:
        int32_t user_id = 0;

        uint8_t activity = 0;
        std::string activity_desc = "";

        std::string beatmap_checksum = "";

        int64_t current_mods = 0;
        uint8_t play_mode = 0;

        int32_t beatmap_id = 0;

        int64_t ranked_score = 0;

        int32_t total_hits = 0;
        int32_t max_combo = 0;
        float accuracy = 0.0f; // Between 0 and 1 (divide by 100 I guess)
        int32_t play_count = 0;
        int32_t play_time = 0;
        int64_t total_score = 0;

        int32_t count_A = 0;
        int32_t count_S = 0;
        int32_t count_X = 0;  // SS
        int32_t count_SH = 0; // S  with Hidden
        int32_t count_XH = 0; // SS with Hidden

        int32_t rank = 0;
        int16_t pp = 0;

        std::vector<std::pair<int64_t, bool>> acquired_achievements {};

        void recalculate_accuracy(bool is_relax);
        void recalculate_pp(bool is_relax);

        buffer marshal() override;
        void unmarshal(shiro::io::buffer &buffer) override;

        int32_t get_size() override;

    };

}

#endif //SHIRO_USER_STATS_HH
