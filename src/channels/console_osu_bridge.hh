/*
 * shiro - High performance, high quality osu!Bancho C++ re-implementation
 * Copyright (C) 2018-2020 Marc3842h, czapek
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

#ifndef SHIRO_CONSOLE_OSU_BRIDGE_HH
#define SHIRO_CONSOLE_OSU_BRIDGE_HH

#include "../thirdparty/naga.hh"

namespace shiro::channels::bridge {

    void install();

    void callback(std::any& user_data, const naga::log_message& message);

    uint8_t get_permission(naga::log_level level);

}

#endif //SHIRO_CONSOLE_OSU_BRIDGE_HH
