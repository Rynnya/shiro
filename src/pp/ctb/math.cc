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

#include "math.hh"

shiro::pp::ctb::point::point(float x, float y)
    : x(x)
    , y(y)
{};

bool shiro::pp::ctb::point::operator==(const point& other) {
    return this->x == other.x && this->y == other.y;
}

shiro::pp::ctb::point shiro::pp::ctb::math::point_at_distance(std::deque<point> arr, float distance) {
    float current_distance = 0;
    float new_distance = 0;
    int32_t i = 0;

    if (arr.size() < 2) {
        return { 0.0f, 0.0f };
    }

    if (distance == 0) {
        return arr.front();
    }

    if (distance_from_points(arr) <= distance) {
        return arr.back();
    }

    for (; i < arr.size() - 2; i++) {
        float x = (arr[i].x - arr[i + 1].x);
        float y = (arr[i].y - arr[i + 1].y);

        new_distance = std::sqrt(x * x + y * y);
        current_distance += new_distance;

        if (distance <= current_distance) {
            break;
        }
    }

    current_distance -= new_distance;

    if (distance == current_distance) {
        return arr[i];
    }

    float angle = std::atan2(arr[i].y - arr[i + 1].y, arr[i].x - arr[i + 1].y);
    point cart = { (distance - current_distance) * std::cos(angle), (distance - current_distance) * std::sin(angle) };

    if (arr[i].x > arr[i + 1].x) {
        return { (arr[i].x - cart.x), (arr[i].y - cart.y) };
    }

    return { (arr[i].x + cart.x), (arr[i].y + cart.y) };
}

shiro::pp::ctb::point shiro::pp::ctb::math::point_on_line(point p0, point p1, float length) {
    float full_length = std::sqrt(std::pow(p1.x - p0.x, 2) + std::pow(p1.y - p0.y, 2));
    float n = full_length - length;

    if (full_length == 0) {
        full_length = 1;
    }

    float x = (n * p0.x + length * p1.x) / full_length;
    float y = (n * p0.y + length * p1.y) / full_length;

    return { x, y };
}

float shiro::pp::ctb::math::distance_from_points(std::deque<point> arr) {
    float distance = 0;

    for (int32_t i = 1; i < arr.size(); i++) {
        float x_ = arr[i].x - arr[i - 1].x;
        float y_ = arr[i].y - arr[i - 1].y;
        distance += std::sqrt(x_ * x_ + y_ * y_);
    }

    return distance;
}
