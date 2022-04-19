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
#include "../../users/user_punishments.hh"
#include "../../utils/bot_utils.hh"
#include "../../utils/string_utils.hh"
#include "../../utils/time_utils.hh"
#include "silence_command.hh"

using fmt::format;

bool shiro::commands::silence(std::deque<std::string>& args, const std::shared_ptr<shiro::users::user>& user, const std::string& channel) {
    if (args.size() < 2) {
        utils::bot::respond("Usage: !silence <user> <duration>[s,min,h,d,w,m] [reason]", user, channel, true);
        return false;
    }

    if (!roles::manager::has_permission(user, permissions::perms::cmd_silence)) {
        utils::bot::respond(format("Permission denied. ({})", static_cast<uint64_t>(permissions::perms::cmd_silence)), user, channel, true);
        return false;
    }

    int32_t target = users::manager::get_id_by_username(args.at(0));
    std::string target_username = args.at(0);
    std::string reason = "You have been silenced";

    if (target == -1) {
        utils::bot::respond(format("{} could not be found.", target_username), user, channel, true);
        return false;
    }

    if (target == 1) {
        utils::bot::respond("Don\'t try to silence robots, silly human!", user, channel, true);
        return false;
    }

    std::optional<int32_t> parsed_time = utils::time::parse_time_string(args.at(1));

    if (!parsed_time.has_value()) {
        utils::bot::respond("Unable to convert duration into valid unsigned integer.", user, channel, true);
        return false;
    }

    int32_t time = parsed_time.value();

    // Remove username and duration
    args.pop_front();
    args.pop_front();

    if (!args.empty()) {
        reason = format("{}", fmt::join(args, " "));
    }

    users::punishments::silence(target, user->user_id, time, reason);
    utils::bot::respond(format("{} has been silenced for {} seconds.", target_username, time), user, channel, true);
    return true;
}
