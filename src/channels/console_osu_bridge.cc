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

#include "../bot/bot.hh"
#include "../users/user_manager.hh"
#include "../utils/bot_utils.hh"
#include "../utils/osu_permissions.hh"
#include "channel_manager.hh"
#include "console_osu_bridge.hh"

void shiro::channels::bridge::install() {
    naga::add_callback("osu! bridge", channels::bridge::callback, naga::log_level::info);
}

void shiro::channels::bridge::callback(std::any& user_data, const naga::log_message& message) {
    std::shared_ptr<users::user> bot_user = bot::bot_user;

    if (bot_user == nullptr) {
        return;
    }

    io::osu_writer writer;
    io::osu_writer future_writer;

    io::layouts::message ingame_message;
    ingame_message.sender = config::bot::name;
    ingame_message.sender_id = 1;
    ingame_message.channel = "#console";
    ingame_message.content = message.message;

    io::layouts::user_presence bot_presence = bot_user->presence;
    bot_presence.permissions = get_permission(message.level);

    writer.user_presence(bot_presence);
    writer.send_message(ingame_message);

    future_writer.user_presence(bot_user->presence);

    for (const std::shared_ptr<users::user> &user : channels::manager::get_users_in_channel("#console")) {
        if (user == nullptr || user->user_id == 1) {
            continue;
        }

        user->queue.enqueue(writer);
        user->queue.enqueue_next(future_writer);
    }
}

uint8_t shiro::channels::bridge::get_permission(naga::log_level level) {
    switch (level) {
        case naga::log_level::info: {
            return static_cast<uint8_t>(utils::osu_permissions::supporter);
        }
        case naga::log_level::error: {
            return static_cast<uint8_t>(utils::osu_permissions::bat);
        }
        case naga::log_level::fatal: {
            return static_cast<uint8_t>(utils::osu_permissions::peppy);
        }
        default: {
            return static_cast<uint8_t>(utils::osu_permissions::friend_);
        }
    }
}
