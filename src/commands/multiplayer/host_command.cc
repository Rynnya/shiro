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

#include "../../multiplayer/match_manager.hh"
#include "../../thirdparty/fmt/format.hh"
#include "../../users/user_manager.hh"
#include "../../utils/bot_utils.hh"
#include "../../utils/string_utils.hh"
#include "host_command.hh"

bool shiro::commands_mp::host(std::deque<std::string>& args, const std::shared_ptr<shiro::users::user>& user, const std::string& channel) {
    if (args.size() < 1) {
        utils::bot::respond("Usage: !mp host <username>", user, channel, true);
        return true;
    }

    if (!shiro::multiplayer::match_manager::in_match(user)) {
        utils::bot::respond("You must be in room to perform this action!", user, channel, true);
        return true;
    }

    std::shared_ptr<users::user> target = shiro::users::manager::get_user_by_username(args.at(0));

    if (target == nullptr || target->hidden) {
        utils::bot::respond("User not found :c", user, channel, true);
        return true;
    }

    std::optional<io::layouts::multiplayer_match> optional = shiro::multiplayer::match_manager::get_match(user);

    if (!optional.has_value()) {
        return true;
    }

    io::layouts::multiplayer_match match = *optional;
    if (match.host_id == user->user_id) {

        for (size_t i = 0; i < match.multi_slot_id.size(); i++) {
            int32_t user_id = match.multi_slot_id.at(i);

            if (user_id == target->user_id) {
                match.host_id = user_id;
                match.send_update(true);

                io::osu_writer writer;
                writer.match_transfer_host();

                target->queue.enqueue(writer);
                utils::bot::respond(fmt::format("Host was transfered to {}", target->presence.username), user, channel, true);
                return true;
            }
        }

        utils::bot::respond("User not found in current match :c", user, channel, true);
        return true;
    }

    utils::bot::respond("You must be a host to perform this action!", user, channel, true);
    return true;
}
