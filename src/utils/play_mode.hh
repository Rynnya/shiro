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

#ifndef SHIRO_PLAY_MODE_HH
#define SHIRO_PLAY_MODE_HH

#include <cstdint>
#include <string>

namespace shiro::utils {

    enum class play_mode : uint8_t {
        standard = 0,
        taiko = 1,
        fruits = 2,
        mania = 3

    };

    std::string play_mode_to_string(play_mode mode, bool long_name = true);

}

#endif  //SHIRO_PLAY_MODE_HH
