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
#include "../../users/user_punishments.hh"
#include "../../utils/bot_utils.hh"
#include "../../utils/string_utils.hh"
#include "ban_command.hh"

using fmt::format;

bool shiro::commands::ban(std::deque<std::string>& args, const std::shared_ptr<shiro::users::user>& user, const std::string& channel) {
    if (args.size() < 2) {
        utils::bot::respond("Usage: !ban <user> <reason>", user, channel, true);
        return false;
    }

    if (!shiro::users::manager::has_permissions(user, permissions::perms::ban_users)) {
        utils::bot::respond("Permission denied. (Ban permission required)", user, channel, true);
        return false;
    }

    int32_t target = users::manager::get_id_by_username(args.at(0));
    std::string target_username = args.at(0);

    if (target == -1) {
        utils::bot::respond(format("{} could not be found.", target_username), user, channel, true);
        return false;
    }

    if (target == 1) {
        utils::bot::respond("Don\'t try to ban robots, silly human!", user, channel, true);
        return false;
    }

    // Remove username
    args.pop_front();
    std::string reason = format("{}", fmt::join(args, " "));

    users::punishments::restrict(target, user->user_id, reason);
    utils::bot::respond(format("{} has been banned for '{}'.", target_username, reason), user, channel, true);
    return true;
}
