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
#include "../../utils/bot_utils.hh"
#include "lock_command.hh"

bool shiro::commands_mp::lock(std::deque<std::string>& args, std::shared_ptr<shiro::users::user> user, std::string channel)
{
    if (!shiro::multiplayer::match_manager::in_match(user))
    {
        utils::bot::respond("You must be in room to perform this action!", user, channel, true);
        return true;
    }

    shiro::multiplayer::match_manager::iterate([&user, &channel](shiro::io::layouts::multiplayer_match& match) -> bool
    {
        auto iterator = std::find(match.multi_slot_id.begin(), match.multi_slot_id.end(), user->user_id);

        if (iterator == match.multi_slot_id.end())
            return false;

        if (match.host_id == user->user_id)
        {
            match.slots_locked = true;
            utils::bot::respond("Slots was locked.", user, channel, true);
            return true;
        }

        utils::bot::respond("You must be a host to perform this action!", user, channel, true);
        return true;
    });

    return true;
}
