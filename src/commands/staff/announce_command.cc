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

#include "../../permissions/role_manager.hh"
#include "../../thirdparty/fmt/format.hh"
#include "../../users/user_manager.hh"
#include "../../utils/bot_utils.hh"
#include "../../utils/string_utils.hh"
#include "announce_command.hh"

using fmt::format;

bool shiro::commands::announce(std::deque<std::string>& args, const std::shared_ptr<shiro::users::user>& user, const std::string& channel) {
    if (args.empty()) {
        utils::bot::respond("Usage: !announce [user] <announcement>", user, channel, true);
        return false;
    }

    if (!roles::manager::has_permission(user, permissions::perms::cmd_announce)) {
        utils::bot::respond(format("Permission denied. ({})", static_cast<uint64_t>(permissions::perms::cmd_announce)), user, channel, true);
        return false;
    }

    std::shared_ptr<users::user> target = nullptr;

    if (args.size() >= 2) {
        target = users::manager::get_user_by_username(args.at(0));

        if (target != nullptr && users::manager::is_online(target)) {
            args.pop_front();
        }
    }

    io::osu_writer writer;
    writer.announce(format("{}", fmt::join(args, " ")));

    if (target != nullptr) {
        target->queue.enqueue(writer);
        utils::bot::respond(format("Successfully sent a announcement to {}", target->presence.username), user, channel, true);
        return true;
    }

    users::manager::iterate([&writer](std::shared_ptr<users::user> online_user) {
        online_user->queue.enqueue(writer);
    }, true);

    utils::bot::respond("Successfully sent a announcement to everyone.", user, channel, true);
    return true;
}
