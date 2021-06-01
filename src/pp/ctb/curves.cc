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

#include <cmath>

#include "curves.hh"

// Bezier //////////////////////////////////////////////////////////////////////

std::pair<float, float> shiro::pp::ctb::Bezier::point_at_distance(float distance)
{
    if (this->order == 0)
        return std::make_pair(0, 0);

    if (this->order == 1)
        return this->points[0];

    return math::point_at_distance(this->points, distance);
}

void shiro::pp::ctb::Bezier::setup()
{
    if (this->points.size() != 0)
        return;

    std::deque<std::pair<float, float>> sub_points = {};
    for (int i = 0; i < this->curve_points.size(); i++)
    {
        if (i == this->curve_points.size() - 1)
        {
            sub_points.push_back(points[i]);
            bezier(sub_points);
            sub_points.clear();
        }
        else if (sub_points.size() > 1 && this->curve_points[i] == sub_points[sub_points.size() - 1])
        {
            bezier(sub_points);
            sub_points.clear();
        }

        sub_points.push_back(this->curve_points[i]);
    }
}

void shiro::pp::ctb::Bezier::bezier(std::deque<std::pair<float, float>> points)
{
    int order = points.size();
    float step = 0.25 / constants::SLIDER_QUALITY / order;
    float i = 0;
    int n = order - 1;

    float x, y;

    while (i < 1 + step)
    {
        x = 0;
        y = 0;

        for (int p = 0; p < n + 1; p++)
        {
            float a = cpn(p, n) * std::pow(1 - p, n - p) * std::pow(i, p);
            x += a * points[i].first;
            y += a * points[i].second;
        }

        this->points.push_back(std::make_pair(x, y));
        i += step;
    }
}

float shiro::pp::ctb::Bezier::cpn(int p, int n)
{
    if (p < 0 || p > n)
        return 0;

    p = std::min(p, n - p);
    float out = 1;
    for (int i = 1; i < p + 1; i++)
        out = out * (n - p + i) / i;

    return out;
}

// Catmull /////////////////////////////////////////////////////////////////////

std::pair<float, float> shiro::pp::ctb::Catmull::point_at_distance(float distance)
{
    if (this->order == 0)
        return std::make_pair(0, 0);

    if (this->order == 1)
        return this->points[0];

    return math::point_at_distance(this->points, distance);
}

void shiro::pp::ctb::Catmull::setup()
{
    if (this->points.size() != 0)
        return;

    int x = 0;
    float t = 0;
    std::pair<float, float> v1, v2, v3, v4;
    for (; x < this->order - 1; x++)
    {
        t = 0;
        while (t < this->step + 1)
        {
            if (x >= 1)
                v1 = this->curve_points[x - 1];
            else
                v1 = this->curve_points[x];

            v2 = this->curve_points[x];

            if (x + 1 < this->order)
                v3 = this->curve_points[x + 1];
            else
                v3 = appvec(v2, 1, appvec(v2, -1, v1));

            if (x + 2 < this->order)
                v4 = this->curve_points[x + 2];
            else
                v4 = appvec(v3, 1, appvec(v3, -1, v2));

            std::pair<float, float> point = get_point({ v1, v2, v3, v4 }, t);
            this->points.push_back(point);
            t += this->step;
        }
    }
}

std::pair<float, float> shiro::pp::ctb::Catmull::get_point(std::deque<std::pair<float, float>> points, float length)
{
    std::deque<float> x_points = { points[0].first, points[1].first, points[2].first, points[3].first };
    std::deque<float> y_points = { points[0].second, points[1].second, points[2].second, points[3].second };
    float x = catmull(x_points, length);
    float y = catmull(y_points, length);
    return std::make_pair(x, y);
}

float shiro::pp::ctb::Catmull::catmull(std::deque<float> p, float length)
{
    // Holy crap who made this?
    return 0.5 * (
        (2 * p[1]) +
        (-p[0] + p[2]) * length +
        (2 * p[0] - 5 * p[1] + 4 * p[2] - p[3]) * std::pow(length, 2) +
        (-p[0] + 3 * p[1] - 3 * p[2] + p[3]) * std::pow(length, 3));
}

// Perfect /////////////////////////////////////////////////////////////////////

std::pair<float, float> shiro::pp::ctb::Perfect::point_at_distance(float distance)
{
    float radians = distance / this->radius;
    return rotate(this->curve_points[0], radians);
}

void shiro::pp::ctb::Perfect::setup()
{
    auto [x, y, r] = get_circum_circle();

    this->cx = x;
    this->cy = y;
    this->radius = r;

    if (((this->curve_points[1].first - this->curve_points[0].first)
        * (this->curve_points[2].second - this->curve_points[0].second)
        - (this->curve_points[1].second - this->curve_points[0].second)
        * (this->curve_points[2].first - this->curve_points[0].first))
        < 0)
        this->radius *= -1;
}

std::tuple<float, float, float> shiro::pp::ctb::Perfect::get_circum_circle()
{
    float d = 2 * (this->curve_points[0].first * (this->curve_points[1].second - this->curve_points[2].second)
        + this->curve_points[1].first * (this->curve_points[2].second - this->curve_points[0].second)
        + this->curve_points[2].first * (this->curve_points[0].second - this->curve_points[1].second));

    if (d == 0)
        throw std::exception("Invalid circle! Unable to chose angle.");

    float ux = ((std::pow(this->curve_points[0].first, 2) + std::pow(this->curve_points[0].second, 2))
        * (this->curve_points[1].first - this->curve_points[2].first)
        + (std::pow(this->curve_points[1].first, 2) + std::pow(this->curve_points[1].second, 2))
        * (this->curve_points[2].second - this->curve_points[0].second)
        + (std::pow(this->curve_points[2].first, 2) + std::pow(this->curve_points[2].second, 2))
        * (this->curve_points[0].second - this->curve_points[1].second)) / d;

    float uy = ((std::pow(this->curve_points[0].first, 2) + std::pow(this->curve_points[0].second, 2))
        * (this->curve_points[2].first - this->curve_points[1].first)
        + (std::pow(this->curve_points[1].first, 2) + std::pow(this->curve_points[1].second, 2))
        * (this->curve_points[0].first - this->curve_points[2].first)
        + (std::pow(this->curve_points[2].first, 2) + std::pow(this->curve_points[2].second, 2))
        * (this->curve_points[1].first - this->curve_points[0].first)) / d;

    float px = ux - this->curve_points[0].first;
    float py = uy - this->curve_points[0].second;

    float r = std::sqrt(std::pow(px, 2) + std::pow(py, 2));

    return { ux, uy, r };
}

std::pair<float, float> shiro::pp::ctb::Perfect::rotate(std::pair<float, float> obj, float radians)
{
    float cos = std::cos(radians);
    float sin = std::sin(radians);

    return std::pair<float, float>
    (
        (cos * (obj.first - cx)) - (sin * (obj.second - cy)) + cx,
        (sin * (obj.first - cx)) + (cos * (obj.second - cy)) + cy
    );
}
