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

template <>
bool shiro::utils::strings::evaluate(const std::string& src, bool& value) noexcept {
    std::string result;

    std::transform(src.begin(), src.end(), std::back_inserter(result), ::tolower);
    value = (result == "true" || result == "1");

    return true;
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
