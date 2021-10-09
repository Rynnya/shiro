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

bool shiro::commands_mp::size(std::deque<std::string>& args, std::shared_ptr<shiro::users::user> user, std::string channel)
{
    if (!shiro::multiplayer::match_manager::in_match(user))
    {
        utils::bot::respond("You must be in room to perform this action!", user, channel, true);
        return true;
    }

    if (args.size() < 1)
    {
        utils::bot::respond("Usage: !mp size <number>", user, channel, true);
        return true;
    }

    int32_t size;
    if (!utils::strings::safe_int(args.at(0), size))
    {
        utils::bot::respond("You must provide a number!", user, channel, true);
        utils::bot::respond("Usage: !mp size <number>", user, channel, true);
        return true;
    }

    if (size < 1 || size > 16)
    {
        utils::bot::respond("Number must be between 1 and 16.", user, channel, true);
        return true;
    }

    shiro::multiplayer::match_manager::iterate([&user, &channel, size](shiro::io::layouts::multiplayer_match& match) -> bool
    {
        auto iterator = std::find(match.multi_slot_id.begin(), match.multi_slot_id.end(), user->user_id);

        if (iterator == match.multi_slot_id.end())
            return false;

        if (match.host_id == user->user_id)
        {
            std::shared_ptr<users::user> kicking_user = nullptr;

            for (int32_t slot_id = 15; slot_id >= size; slot_id--)
            {
                int32_t kicking_user_id = match.multi_slot_id.at(slot_id);

                if (kicking_user_id != -1 && kicking_user_id != user->user_id)
                    kicking_user = users::manager::get_user_by_id(kicking_user_id);

                uint8_t& slot_status = match.multi_slot_status.at(slot_id);
                slot_status = static_cast<uint8_t>(utils::slot_status::locked);

                if (kicking_user == nullptr)
                    continue;

                // Kick the player outside of #iterate to prevent dead lock
                shiro::multiplayer::match_manager::leave_match(kicking_user);

                io::osu_writer writer;
                writer.channel_revoked("#multiplayer");
                writer.announce("You have been kicked from the match!");

                kicking_user->queue.enqueue(writer);
            }

            match.send_update(true);
            return true;
        }

        utils::bot::respond("You must be a host to perform this action!", user, channel, true);
        return true;
    });

    return true;
}
