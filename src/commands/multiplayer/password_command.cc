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
#include "password_command.hh"

bool shiro::commands_mp::password(std::deque<std::string>& args, std::shared_ptr<shiro::users::user> user, std::string channel) {
    if (args.size() < 1) {
        utils::bot::respond("Usage: !mp password <password>", user, channel, true);
        return true;
    }

    if (!shiro::multiplayer::match_manager::in_match(user)) {
        utils::bot::respond("You must be in room to perform this action!", user, channel, true);
        return true;
    }

    std::string password = args.at(0);

    shiro::multiplayer::match_manager::iterate([&user, &channel, &password](shiro::io::layouts::multiplayer_match& match) -> bool {
        auto iterator = std::find(match.multi_slot_id.begin(), match.multi_slot_id.end(), user->user_id);

        if (iterator == match.multi_slot_id.end()) {
            return false;
        }

        if (match.host_id == user->user_id) {
            if (match.game_password == password) {
                return true;
            }

            match.game_password = password;
            match.send_update(true);

            io::osu_writer writer;
            writer.match_change_password(password);

            for (size_t i = 0; i < match.multi_slot_id.size(); i++) {
                if (match.multi_slot_id.at(i) == -1) {
                    continue;
                }

                std::shared_ptr<users::user> lobby_user = users::manager::get_user_by_id(match.multi_slot_id.at(i));

                if (lobby_user == nullptr) {
                    continue;
                }

                lobby_user->queue.enqueue(writer);
            }

            utils::bot::respond("Password was changed.", user, channel, true);
            return true;
        }

        utils::bot::respond("You must be a host to perform this action!", user, channel, true);
        return true;
    });

    return true;
}
