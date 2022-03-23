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

#ifndef SHIRO_CATCH_THE_PP_MATH_HH
#define SHIRO_CATCH_THE_PP_MATH_HH

#include <cmath>
#include <deque>

namespace shiro::pp::ctb {

    // replacement for std::pair<float, float>
    class point {
    public:
        point(double x = 0.0, double y = 0.0);
        bool operator==(const point& other);

        double x = 0.0;
        double y = 0.0;
    };

    namespace constants {

        static constexpr double STAR_SCALING_FACTOR = 0.145;
        static constexpr double STRAIN_STEP = 750.0;
        static constexpr double DECAY_WEIGHT = 0.94;
        static constexpr double DECAY_BASE = 0.2;
        static constexpr double ABSOLUTE_PLAYER_POSITIONING_ERROR = 16.0;
        static constexpr double NORMALIZED_HITOBJECT_RADIUS = 41.0;
        static constexpr double DIRECTION_CHANGE_BONUS = 12.5;
        static constexpr double SLIDER_QUALITY = 50.0;
    }

    class math {
    public:
        static point point_at_distance(std::deque<point> arr, double distance);
        static point point_on_line(point p0, point p1, double length);
        static float distance_from_points(std::deque<point> arr);

        template <typename T>
        constexpr static int32_t sign(T val) {
            return (T(0) < val) - (val < T(0));
        }
    };
}

#endif
