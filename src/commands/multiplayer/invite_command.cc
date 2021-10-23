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
#include "../../utils/curler.hh"
#include "../../utils/osu_client.hh"
#include "invite_command.hh"

bool shiro::commands_mp::invite(std::deque<std::string>& args, std::shared_ptr<shiro::users::user> user, std::string channel)
{
    if (!shiro::multiplayer::match_manager::in_match(user))
    {
        utils::bot::respond("You must be in room to perform this action!", user, channel, true);
        return true;
    }

    if (args.size() < 1)
    {
        utils::bot::respond("Usage: !mp invite <username>", user, channel, true);
        return true;
    }

    std::shared_ptr<users::user> target = shiro::users::manager::get_user_by_username(args.at(0));

    if (target == nullptr || target->hidden)
    {
        utils::bot::respond("User not found :c", user, channel, true);
        return true;
    }

    if (target->client_type == +utils::clients::osu_client::aschente)
    {
        utils::bot::respond("Thanks for the invite but I have to decline :)", user, target->presence.username, true);
        return true;
    }

    std::optional<io::layouts::multiplayer_match> optional = shiro::multiplayer::match_manager::get_match(user);

    // Some how user left lobby after invite almost instantly
    if (!optional.has_value())
        return true;

    io::layouts::multiplayer_match match = *optional;
    std::string url = "osump://" + std::to_string(match.match_id) + "/";
    auto iterator = std::find(match.multi_slot_id.begin(), match.multi_slot_id.end(), user->user_id);

    // If the sending user is in the lobby themselves, we can send the password without a problem
    if (iterator != match.multi_slot_id.end() && !match.game_password.empty())
        url.append(utils::curl::escape_url(match.game_password));

    io::osu_writer writer;
    io::layouts::message message;

    message.sender = user->presence.username;
    message.sender_id = user->user_id;

    message.content = "Hey! Come and join my multiplayer room: [" + url + " " + match.game_name + "]";
    message.channel = target->presence.username;

    writer.send_message(message);
    target->queue.enqueue(writer);

    return true;
}
