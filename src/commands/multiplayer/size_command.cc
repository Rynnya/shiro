/*
 * shiro - High performance, high quality osu!Bancho C++ re-implementation
 * Copyright (C) 2021 Rynnya
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
#include "../../users/user_manager.hh"
#include "../../utils/bot_utils.hh"
#include "../../utils/slot_status.hh"
#include "../../utils/string_utils.hh"
#include "size_command.hh"

bool shiro::commands_mp::size(std::deque<std::string>& args, std::shared_ptr<shiro::users::user> user, std::string channel) {
    if (args.size() < 1) {
        utils::bot::respond("Usage: !mp size <number>", user, channel, true);
        return true;
    }

    if (!shiro::multiplayer::match_manager::in_match(user)) {
        utils::bot::respond("You must be in room to perform this action!", user, channel, true);
        return true;
    }

    int32_t size = 0;
    if (!utils::strings::evaluate(args.at(0), size)) {
        utils::bot::respond("You must provide a number!", user, channel, true);
        utils::bot::respond("Usage: !mp size <number>", user, channel, true);
        return true;
    }

    if (size < 1 || size > 16) {
        utils::bot::respond("Number must be between 1 and 16.", user, channel, true);
        return true;
    }

    std::vector<std::shared_ptr<users::user>> removed_users = {};

    shiro::multiplayer::match_manager::iterate([&removed_users, &user, &channel, size = static_cast<size_t>(size)](shiro::io::layouts::multiplayer_match& match) -> bool {
        auto iterator = std::find(match.multi_slot_id.begin(), match.multi_slot_id.end(), user->user_id);

        if (iterator == match.multi_slot_id.end()) {
            return false;
        }

        if (match.host_id == user->user_id) {
            for (size_t i = 15; i >= size; i--) {
                int32_t kicking_user_id = match.multi_slot_id.at(i);

                if (kicking_user_id != -1 && kicking_user_id != user->user_id) {
                    removed_users.push_back(users::manager::get_user_by_id(kicking_user_id));
                }

                uint8_t& slot_status = match.multi_slot_status.at(i);
                slot_status = static_cast<uint8_t>(utils::slot_status::locked);
            }

            // Unlock every slot that not in range but locked
            for (size_t i = 0; i < size; i++) {
                uint8_t& slot_status = match.multi_slot_status.at(i);
                if (slot_status == static_cast<uint8_t>(utils::slot_status::locked)) {
                    slot_status = static_cast<uint8_t>(utils::slot_status::open);
                }
            }

            match.send_update(true);
            utils::bot::respond("Size was changed to " + std::to_string(size), user, channel, true);
            return true;
        }

        utils::bot::respond("You must be a host to perform this action!", user, channel, true);
        return true;
    });

    // Kick players outside of #iterate to prevent dead lock
    if (removed_users.size() == 0) {
        return true;
    }

    for (const std::shared_ptr<shiro::users::user>& user : removed_users) {
        if (user == nullptr) {
            continue; // get_user_by_id might return nullptr
        }

        shiro::multiplayer::match_manager::leave_match(user);

        shiro::io::osu_writer writer;
        writer.channel_revoked("#multiplayer");
        writer.announce("You have been kicked from the match!");

        user->queue.enqueue(writer);
    }

    return true;
}
