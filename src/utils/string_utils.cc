/*
 * shiro - High performance, high quality osu!Bancho C++ re-implementation
 * Copyright (C) 2018-2020 Marc3842h, czapek
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
#include "string_utils.hh"

bool shiro::utils::strings::safe_int(const std::string& src, int& num)
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

int shiro::utils::strings::safe_int(const std::string& src)
{
    int num;
    if (safe_int(src, num))
        return num;
    return -1;
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

bool shiro::utils::strings::safe_long_long(const std::string& src, long long& num)
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

long long shiro::utils::strings::safe_long_long(const std::string& src)
{
    long long num;
    if (safe_long_long(src, num))
        return num;
    return -1;
}

bool shiro::utils::strings::to_bool(std::string src)
{
    std::transform(src.begin(), src.end(), src.begin(), ::tolower);
    return src == "true";
}

std::string shiro::utils::strings::to_string(bool src)
{
    return src ? "true" : "false";
}
