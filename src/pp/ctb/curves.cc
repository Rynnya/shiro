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

#include <cmath>
#include <stdexcept>
#include <tuple>

#include "curves.hh"

// Bezier //////////////////////////////////////////////////////////////////////

shiro::pp::ctb::point shiro::pp::ctb::bezier::point_at_distance(float distance) {
    if (this->order == 0) {
        return { 0.0f, 0.0f };
    }

    if (this->order == 1) {
        return this->points.front();
    }

    return math::point_at_distance(this->points, distance);
}

void shiro::pp::ctb::bezier::setup() {
    if (this->points.size() != 0) {
        return;
    }

    std::vector<point> sub_points = {};
    for (size_t i = 0; i < this->curve_points.size(); i++) {

        if (i == this->curve_points.size() - 1) {
            sub_points.push_back(points[i]);
            bezier_(sub_points);
            sub_points.clear();
        }
        else if (sub_points.size() > 1 && this->curve_points[i] == sub_points[sub_points.size() - 1]) {
            bezier_(sub_points);
            sub_points.clear();
        }

        sub_points.push_back(this->curve_points[i]);
    }
}

void shiro::pp::ctb::bezier::bezier_(std::vector<point> points) {
    int32_t order = points.size();
    float step = 0.25 / constants::SLIDER_QUALITY / order;
    float i = 0;
    int32_t n = order - 1;

    while (i < 1 + step) {
        float x = 0;
        float y = 0;

        for (int32_t p = 0; p < n + 1; p++) {
            float a = cpn(p, n) * std::pow(1 - p, n - p) * std::pow(i, p);
            x += a * points[i].x;
            y += a * points[i].y;
        }

        this->points.push_back({ x, y });
        i += step;
    }
}

float shiro::pp::ctb::bezier::cpn(int32_t p, int32_t n) {
    if (p < 0 || p > n) {
        return 0;
    }

    p = std::min(p, n - p);
    float out = 1;

    for (int32_t i = 1; i < p + 1; i++) {
        out *= (n - p + i) / i;
    }

    return out;
}

// Catmull /////////////////////////////////////////////////////////////////////

shiro::pp::ctb::point shiro::pp::ctb::catmull::point_at_distance(float distance) {
    if (this->order == 0) {
        return { 0.0f, 0.0f };
    }

    if (this->order == 1) {
        return this->points[0];
    }

    return math::point_at_distance(this->points, distance);
}

void shiro::pp::ctb::catmull::setup() {
    if (this->points.size() != 0) {
        return;
    }

    point v1, v2, v3, v4;
    for (size_t x = 0; x < this->order - 1; x++) {
        float t = 0;
        while (t < this->step + 1) {
            v1 = x >= 1 ? this->curve_points[x - 1] : this->curve_points[x];
            v2 = this->curve_points[x];
            v3 = x + 1 < this->order ? this->curve_points[x + 1] : appvec(v2, 1, appvec(v2, -1, v1));
            v4 = x + 2 < this->order ? this->curve_points[x + 2] : appvec(v3, 1, appvec(v3, -1, v2));

            this->points.push_back(get_point({ v1, v2, v3, v4 }, t));
            t += this->step;
        }
    }
}

shiro::pp::ctb::point shiro::pp::ctb::catmull::get_point(std::array<point, 4> points, float length) {
    double x = catmull_({ points[0].x, points[1].x, points[2].x, points[3].x }, length);
    double y = catmull_({ points[0].y, points[1].y, points[2].y, points[3].y }, length);

    return { x, y };
}

double shiro::pp::ctb::catmull::catmull_(std::array<double, 4> points, float length) {
    // Holy crap who made this?
    return 0.5 * (
        (2 * points[1]) +
        (-points[0] + points[2]) * length +
        (2 * points[0] - 5 * points[1] + 4 * points[2] - points[3]) * std::pow(length, 2) +
        (-points[0] + 3 * points[1] - 3 * points[2] + points[3]) * std::pow(length, 3));
}

// Perfect /////////////////////////////////////////////////////////////////////

shiro::pp::ctb::point shiro::pp::ctb::perfect::point_at_distance(float distance) {
    float radians = distance / this->radius;
    return rotate(this->curve_points[0], radians);
}

void shiro::pp::ctb::perfect::setup() {
    auto [x, y, r] = get_circum_circle();

    this->cx = x;
    this->cy = y;
    this->radius = r;

    if (((this->curve_points[1].x - this->curve_points[0].x)
        * (this->curve_points[2].y - this->curve_points[0].y)
        - (this->curve_points[1].y - this->curve_points[0].y)
        * (this->curve_points[2].x - this->curve_points[0].x))
        < 0) {
        this->radius *= -1;
    }
}

std::tuple<float, float, float> shiro::pp::ctb::perfect::get_circum_circle() {
    float d = 2 * (this->curve_points[0].x * (this->curve_points[1].y - this->curve_points[2].y)
        + this->curve_points[1].x * (this->curve_points[2].y - this->curve_points[0].y)
        + this->curve_points[2].x * (this->curve_points[0].y - this->curve_points[1].y));

    if (d == 0) {
        throw std::invalid_argument("Invalid circle! Unable to chose angle.");
    }

    float ux = ((std::pow(this->curve_points[0].x, 2) + std::pow(this->curve_points[0].y, 2))
        * (this->curve_points[1].x - this->curve_points[2].x)
        + (std::pow(this->curve_points[1].x, 2) + std::pow(this->curve_points[1].y, 2))
        * (this->curve_points[2].y - this->curve_points[0].y)
        + (std::pow(this->curve_points[2].x, 2) + std::pow(this->curve_points[2].y, 2))
        * (this->curve_points[0].y - this->curve_points[1].y)) / d;

    float uy = ((std::pow(this->curve_points[0].x, 2) + std::pow(this->curve_points[0].y, 2))
        * (this->curve_points[2].x - this->curve_points[1].x)
        + (std::pow(this->curve_points[1].x, 2) + std::pow(this->curve_points[1].y, 2))
        * (this->curve_points[0].x - this->curve_points[2].x)
        + (std::pow(this->curve_points[2].x, 2) + std::pow(this->curve_points[2].y, 2))
        * (this->curve_points[1].x - this->curve_points[0].x)) / d;

    float px = ux - this->curve_points[0].x;
    float py = uy - this->curve_points[0].y;

    float r = std::sqrt(std::pow(px, 2) + std::pow(py, 2));

    return { ux, uy, r };
}

shiro::pp::ctb::point shiro::pp::ctb::perfect::rotate(point obj, float radians) {
    float cos = std::cos(radians);
    float sin = std::sin(radians);

    return {
        (cos * (obj.x - cx)) - (sin * (obj.y - cy)) + cx,
        (sin * (obj.x - cx)) + (cos * (obj.y - cy)) + cy
    };
}
