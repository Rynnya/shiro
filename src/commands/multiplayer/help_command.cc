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

#include "../../utils/bot_utils.hh"
#include "help_command.hh"

bool shiro::commands_mp::help(std::deque<std::string>& args, const std::shared_ptr<shiro::users::user>& user, const std::string& channel) {
    utils::bot::respond("Commands:", user, channel, true);
    utils::bot::respond("!mp help - Shows list of commands", user, channel, true);

    utils::bot::respond("!mp abort - Aborts in-game match", user, channel, true);
    utils::bot::respond("!mp host - Transfers host to another player", user, channel, true);
    utils::bot::respond("!mp invite - Sends invite to this lobby", user, channel, true);
    utils::bot::respond("!mp lock - Prohibits players from changing the slot", user, channel, true);
    utils::bot::respond("!mp unlock - Allows players to change the slot", user, channel, true);
    utils::bot::respond("!mp map - Changes current beatmap", user, channel, true);
    utils::bot::respond("!mp password - Changes current password", user, channel, true);
    utils::bot::respond("!mp size - Changes size of lobby, must be between 1 and 16", user, channel, true);

    return true;
}
