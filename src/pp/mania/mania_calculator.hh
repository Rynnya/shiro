/*
 * shiro - High performance, high quality osu!Bancho C++ re-implementation
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

#ifndef SHIRO_MANIA_CALCULATOR_HH
#define SHIRO_MANIA_CALCULATOR_HH

#include <cmath>

#include "../../beatmaps/beatmap.hh"
#include "../../scores/score.hh"

namespace shiro::pp::mania {

    class note {
    public:
        note() :
            contains_data(false),
            key(0),
            start_t(0),
            end_t(0),
            individual_strain(0)
        {};
        note(double key, int32_t start, int32_t end, double ind_strain) :
            contains_data(true),
            key(key),
            start_t(start),
            end_t(end),
            individual_strain(ind_strain)
        {};

        double key;
        int32_t start_t;
        int32_t end_t;
        double overall_strain = 1;
        double individual_strain;
        bool contains_data;

        note& operator=(const note& other) noexcept = default;

        note& operator=(std::nullptr_t) noexcept {
            key = 0;
            start_t = 0;
            end_t = 0;
            individual_strain = 0;
            contains_data = false;
            return *this;
        };

        operator bool() const noexcept {
            return this->contains_data;
        };
    };

    class mania_calculator {
    private:
        float od = 0;
        float stars = 0;
        int32_t mods = 0;
        int32_t keys = 0;
        int32_t score = 0;
        std::vector<note> notes;

        void calculate_stars();
        void parse_file(std::string filename);
        void parse_note(std::string line, int32_t keys);

    public:
        mania_calculator(beatmaps::beatmap beatmap, scores::score score);

        float calculate();
    };

}

#endif //SHIRO_MANIA_CALCULATOR_HH
