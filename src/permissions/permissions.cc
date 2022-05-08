/*
 * shiro - High performance, high quality osu!Bancho C++ re-implementation
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

#include "../config/bancho_file.hh"
#include "../utils/osu_permissions.hh"

#include "permissions.hh"

uint8_t shiro::permissions::get_chat_color(int64_t permissions) {
    // Bot specific color (Only BanchoBot can be pink, this is hardcoded behaviour in osu!)
    if (permissions == 0xDEADCAFE) {
        return static_cast<uint8_t>(utils::osu_permissions::friend_);
    }

    if (permissions & shiro::permissions::perms::manage_users) {
        return static_cast<uint8_t>(utils::osu_permissions::peppy);
    }

    if (permissions & shiro::permissions::perms::api_beatmap) {
        return static_cast<uint8_t>(utils::osu_permissions::bat);
    }

    if (config::bancho::default_supporter) {
        return static_cast<uint8_t>(utils::osu_permissions::supporter);
    }

    return static_cast<uint8_t>(utils::osu_permissions::normal);
}