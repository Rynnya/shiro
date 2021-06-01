/*
 * shiro - High performance, high quality osu!Bancho C++ re-implementation
 * Copyright (C) 2018-2020 Marc3842h, czapek
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

#include <deque>

#include "../../beatmaps/beatmap.hh"
#include "../../scores/score.hh"
#include "math.hh"

namespace shiro::pp::ctb
{
    // Abstract class for every curve
    class curve
    {
    public:
        virtual std::pair<float, float> point_at_distance(float distance) = 0;
    private:
        virtual void setup() = 0;
    };

    class Bezier : public curve
    {
    public:
        Bezier(std::deque<std::pair<float, float>> curve_points) : curve_points(curve_points)
        {
            setup();
        };

        std::deque<std::pair<float, float>> curve_points = {};
        int order = curve_points.size();
        std::deque<std::pair<float, float>> points = {};

        std::pair<float, float> point_at_distance(float distance) override;

    private:
        void setup() override;
        void bezier(std::deque<std::pair<float, float>> points);
        float cpn(int p, int n);
    };

    class Catmull : public curve
    {
    public:
        Catmull(std::deque<std::pair<float, float>> curve_points) : curve_points(curve_points)
        {
            setup();
        };

        std::deque<std::pair<float, float>> curve_points = {};
        int order = curve_points.size();
        float step = 2.5 / constants::SLIDER_QUALITY;
        std::deque<std::pair<float, float>> points = {};

        std::pair<float, float> point_at_distance(float distance) override;

    private:
        void setup() override;
        std::pair<float, float> appvec(std::pair<float, float> self, float value, std::pair<float, float> other)
        {
            return std::make_pair(self.first + value * other.first, self.second + value * other.second);
        }
        std::pair<float, float> get_point(std::deque<std::pair<float, float>> points, float length);
        float catmull(std::deque<float> p, float length);
    };

    class Perfect : public curve
    {
    public:
        Perfect(std::deque<std::pair<float, float>> curve_points) : curve_points(curve_points)
        {
            setup();
        };

        std::deque<std::pair<float, float>> curve_points = {};
        float cx = 0;
        float cy = 0;
        float radius = 0;

        std::pair<float, float> point_at_distance(float distance) override;
    private:
        void setup() override;
        std::tuple<float, float, float> get_circum_circle();
        std::pair<float, float> rotate(std::pair<float, float> obj, float radians);
    };
}

#endif //SHIRO_CATCH_THE_PP_CURVES_HH