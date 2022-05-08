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

#include "../../config/bot_file.hh"
#include "../../permissions/permissions.hh"
#include "../../thirdparty/fmt/format.hh"
#include "../../users/user_manager.hh"
#include "../../utils/bot_utils.hh"
#include "../../utils/string_utils.hh"
#include "clear_command.hh"

using fmt::format;

bool shiro::commands::clear(std::deque<std::string>& args, const std::shared_ptr<shiro::users::user>& user, const std::string& channel) {
    std::shared_ptr<users::user> target = nullptr;

    if (args.size() >= 2) {
        utils::bot::respond("Usage: !clear [user]", user, channel, true);
        return false;
    }

    if (!shiro::users::manager::has_permissions(user, permissions::perms::clear_cmd)) {
        utils::bot::respond("Permission denied. (Global clear permission required)", user, channel, true);
        return false;
    }

    if (args.size() == 1) {
        target = users::manager::get_user_by_username(args.at(0));

        if (target == nullptr) {
            utils::bot::respond(format("{} is currently not online or does not exist.", args.at(0)), user, channel, true);
            return false;
        }
    }

    io::osu_writer writer;

    users::manager::iterate([&writer](std::shared_ptr<users::user> online_user) {
        writer.user_silenced(online_user->user_id);
    });

    if (target != nullptr) {
        target->queue.enqueue(writer);
        utils::bot::respond(format("Successfully cleared chat for {}.", target->presence.username), user, channel, true);
        return true;
    }

    users::manager::iterate([user, &writer](std::shared_ptr<users::user> online_user) {
        online_user->queue.enqueue(writer);
        utils::bot::respond(format("Your chat was cleared by {}.", user->presence.username), online_user, config::bot::name, true);
    }, true);

    utils::bot::respond("Successfully cleared chat for all online users.", user, channel, true);
    return true;
}
