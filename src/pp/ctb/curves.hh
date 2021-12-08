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

#ifndef SHIRO_CATCH_THE_PP_CURVES_HH
#define SHIRO_CATCH_THE_PP_CURVES_HH

#include <array>

#include "../../beatmaps/beatmap.hh"
#include "../../scores/score.hh"
#include "math.hh"

namespace shiro::pp::ctb {

    class abstract_curve {
    public:
        virtual point point_at_distance(float distance) = 0;
    private:
        virtual void setup() = 0;
    };

    class bezier : public abstract_curve {
    public:
        bezier(std::deque<point> curve_points) : curve_points(curve_points) {
            setup();
        };

        std::deque<point> curve_points = {};
        int32_t order = curve_points.size();
        std::deque<point> points = {};

        point point_at_distance(float distance) override;

    private:
        void setup() override;
        void _bezier(std::vector<point> points);
        float cpn(int32_t p, int32_t n);
    };

    class catmull : public abstract_curve {
    public:
        catmull(std::deque<point> curve_points) : curve_points(curve_points) {
            setup();
        };

        std::deque<point> curve_points = {};
        int32_t order = curve_points.size();
        float step = 2.5 / constants::SLIDER_QUALITY;
        std::deque<point> points = {};

        point point_at_distance(float distance) override;

    private:
        void setup() override;
        point appvec(point self, float value, point other) {
            return { self.x + value * other.x, self.y + value * other.y };
        }
        point get_point(std::array<point, 4> points, float length);
        float _catmull(std::array<float, 4> points, float length);
    };

    class perfect : public abstract_curve {
    public:
        perfect(std::deque<point> curve_points) : curve_points(curve_points) {
            setup();
        };

        std::deque<point> curve_points = {};
        float cx = 0;
        float cy = 0;
        float radius = 0;

        point point_at_distance(float distance) override;
    private:
        void setup() override;
        std::tuple<float, float, float> get_circum_circle();
        point rotate(point obj, float radians);
    };
}

#endif //SHIRO_CATCH_THE_PP_CURVES_HH