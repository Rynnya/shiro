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

#ifndef SHIRO_STRING_UTILS_HH
#define SHIRO_STRING_UTILS_HH

#include <vector>
#include <string>

namespace shiro::utils::strings {

    bool safe_int(const std::string& src, int& num);
    int safe_int(const std::string& src);
    bool safe_float(const std::string& src, float& num);
    float safe_float(const std::string& src);
    bool safe_long_long(const std::string& src, long long& num);
    long long safe_long_long(const std::string& src);

    bool to_bool(std::string src);

    std::string to_string(bool src);

}

#endif //SHIRO_STRING_UTILS_HH
