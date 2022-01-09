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

#include <algorithm>
#include <iterator>

#include "string_utils.hh"
#include "../thirdparty/fast_float.hh"

// TODO: Replace with std::from_chars when every major compiler will support floats and doubles
// This might only happend when transfering shiro to C++20

template <>
bool shiro::utils::strings::evaluate(const std::string& src, float& value) noexcept {
    auto [ptr, ec] = fast_float::from_chars(src.data(), src.data() + src.size(), value);
    return ec == std::errc();
}

template <>
bool shiro::utils::strings::evaluate(const std::string& src, double& value) noexcept {
    auto [ptr, ec] = fast_float::from_chars(src.data(), src.data() + src.size(), value);
    return ec == std::errc();
}

template <>
bool shiro::utils::strings::evaluate(const std::string& src, bool& value) noexcept {
    std::string result;

    std::transform(src.begin(), src.end(), std::back_inserter(result), ::tolower);
    value = (result == "true" || result == "1");

    return true;
}

template <>
float shiro::utils::strings::evaluate(const std::string& src) noexcept {
    float result = 0.0f;
    static_cast<void>(fast_float::from_chars(src.data(), src.data() + src.size(), result));
    return result;
}

template <>
double shiro::utils::strings::evaluate(const std::string& src) noexcept {
    double result = 0.0;
    static_cast<void>(fast_float::from_chars(src.data(), src.data() + src.size(), result));
    return result;
}

template <>
bool shiro::utils::strings::evaluate(const std::string& src) noexcept {
    std::string result;

    std::transform(src.begin(), src.end(), std::back_inserter(result), ::tolower);
    return (result == "true" || result == "1");
}

std::string shiro::utils::strings::to_string(bool src) noexcept {
    return src ? "true" : "false";
}
