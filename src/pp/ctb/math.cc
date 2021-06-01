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

#include <string>
#include <cmath>

#include "math.hh"

std::pair<float, float> shiro::pp::ctb::math::point_at_distance(std::deque<std::pair<float, float>> arr, float distance)
{
    float x, y;
    float current_distance = 0;
    float new_distance = 0;
    int i = 0;

    if (arr.size() < 2)
        return std::make_pair(0, 0);

    if (distance == 0)
        return arr[0];

    if (distance_from_points(arr) <= distance)
        return arr[arr.size() - 1];

    for (; i < arr.size() - 2; i++)
    {
        x = (arr[i].first - arr[i + 1].first);
        y = (arr[i].second - arr[i + 1].second);

        new_distance = std::sqrt(x * x + y * y);
        current_distance += new_distance;

        if (distance <= current_distance)
            break;
    }

    current_distance -= new_distance;

    if (distance == current_distance)
        return arr[i];

    float angle = std::atan2(arr[i].second - arr[i + 1].second, arr[i].first - arr[i + 1].second);
    std::pair<float, float> cart = std::make_pair((distance - current_distance) * std::cos(angle), (distance - current_distance) * std::sin(angle));

    if (arr[i].first > arr[i + 1].first)
        return std::make_pair((arr[i].first - cart.first), (arr[i].second - cart.second));
    return std::make_pair((arr[i].first + cart.first), (arr[i].second + cart.second));
}

std::pair<float, float> shiro::pp::ctb::math::point_on_line(std::pair<float, float> p0, std::pair<float, float> p1, float length)
{
    float full_length = std::sqrt(std::pow(p1.first - p0.first, 2) + std::pow(p1.second - p0.second, 2));
    float n = full_length - length;

    if (full_length == 0)
        full_length = 1;

    float x = (n * p0.first + length * p1.first) / full_length;
    float y = (n * p0.second + length * p1.second) / full_length;

    return std::make_pair(x, y);
}

float shiro::pp::ctb::math::distance_from_points(std::deque<std::pair<float, float>> arr)
{
    float distance = 0;
    for (int i = 1; i < arr.size(); i++)
    {
        float x_ = arr[i].first - arr[i - 1].first;
        float y_ = arr[i].second - arr[i - 1].second;
        distance += std::sqrt(x_ * x_ + y_ * y_);
    }
    return distance;
}
