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
#include <climits>
#include <cmath> // for some reasons infinity declared here
#include "string_utils.hh"

bool shiro::utils::strings::safe_uchar(const std::string& src, uint8_t& num)
{
    int32_t parse;
    if (!safe_int(src, parse))
        return false;

    num = static_cast<uint8_t>(parse);
    return true;
}

uint8_t shiro::utils::strings::safe_uchar(const std::string& src)
{
    int32_t parse;
    if (!safe_int(src, parse))
        return 0; // unsigned

    return static_cast<uint8_t>(parse);
}

bool shiro::utils::strings::safe_int(const std::string& src, int32_t& num)
{
    const char* ptr = src.c_str();
    char* end_ptr;
    const long ans = std::strtol(ptr, &end_ptr, 10);

    if (ptr == end_ptr)
        return false;

    if (ans == INT_MAX)
        return false; // Almost NEVER this happend

    num = ans;
    return true;
}

int32_t shiro::utils::strings::safe_int(const std::string& src)
{
    int32_t num;
    if (safe_int(src, num))
        return num;
    return -1;
}

bool shiro::utils::strings::safe_uint(const std::string& src, uint32_t& num)
{
    const char* ptr = src.c_str();
    char* end_ptr;
    const unsigned long ans = std::strtoul(ptr, &end_ptr, 10);

    if (ptr == end_ptr)
        return false;

    if (ans == UINT_MAX)
        return false; // Almost NEVER this happend

    num = ans;
    return true;
}

uint32_t shiro::utils::strings::safe_uint(const std::string& src)
{
    uint32_t num;
    if (safe_uint(src, num))
        return num;
    return 0; // unsigned
}

bool shiro::utils::strings::safe_float(const std::string& src, float& num)
{
    const char* ptr = src.c_str();
    char* end_ptr;
    const float ans = std::strtof(ptr, &end_ptr);

    if (ptr == end_ptr)
        return false;

    if (ans == INFINITY)
        return false;

    num = ans;
    return true;
}

float shiro::utils::strings::safe_float(const std::string& src)
{
    float num;
    if (safe_float(src, num))
        return num;
    return -1;
}

bool shiro::utils::strings::safe_ll(const std::string& src, int64_t& num)
{
    const char* ptr = src.c_str();
    char* end_ptr;
    const long long ans = std::strtoll(ptr, &end_ptr, 10);

    if (ptr == end_ptr)
        return false;

    if (ans == LLONG_MAX)
        return false; // Almost NEVER this happend

    num = ans;
    return true;
}

int64_t shiro::utils::strings::safe_ll(const std::string& src)
{
    int64_t num;
    if (safe_ll(src, num))
        return num;
    return -1;
}

bool shiro::utils::strings::to_bool(std::string src)
{
    std::transform(src.begin(), src.end(), src.begin(), ::tolower);
    return src == "true" || src == "1";
}

std::string shiro::utils::strings::to_string(bool src)
{
    return src ? "true" : "false";
}
