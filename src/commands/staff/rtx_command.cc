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

#include "../../permissions/permissions.hh"
#include "../../thirdparty/fmt/format.hh"
#include "../../users/user_manager.hh"
#include "../../utils/bot_utils.hh"
#include "../../utils/string_utils.hh"
#include "rtx_command.hh"

using fmt::format;

bool shiro::commands::rtx(std::deque<std::string>& args, const std::shared_ptr<shiro::users::user>& user, const std::string& channel) {
    if (args.empty()) {
        utils::bot::respond("Usage: !rtx <target> [reason]", user, channel, true);
        return false;
    }

    if (!shiro::users::manager::has_permissions(user, permissions::perms::rtx_cmd)) {
        utils::bot::respond("Permission denied. (RTX permission required)", user, channel, true);
        return false;
    }

    std::shared_ptr<users::user> target = users::manager::get_user_by_username(args.at(0));

    if (target == nullptr || !users::manager::is_online(target)) {
        utils::bot::respond(format("{} is currently not online or does not exist.", args.at(0)), user, channel, true);
        return false;
    }

    // No reason has been provided, set default reason
    if (args.size() == 1) {
        args.emplace_back(format("rtx'ed by {}", user->presence.username));
    }

    args.pop_front();

    io::osu_writer writer;
    writer.rtx(format("{}", fmt::join(args, " ")));

    target->queue.enqueue(writer);
    utils::bot::respond(format("{} has been successfully rtx'ed.", target->presence.username), user, channel, true);
    return true;
}
