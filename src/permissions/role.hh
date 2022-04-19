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

#ifndef SHIRO_ROLE_HH
#define SHIRO_ROLE_HH

#include <cstdint>
#include <string>

namespace shiro::permissions {

    class role {
    public:
        int32_t id = 0;
        int64_t weight = 0;
        std::string name = "";
        int64_t permissions = 0;
        int32_t color = 0;

        role(int32_t id, int64_t weight, std::string name, int64_t permissions, int32_t color);

    };

}

#endif //SHIRO_ROLE_HH
