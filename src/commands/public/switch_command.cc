/*
 * shiro - High performance, high quality osu!Bancho C++ re-implementation
 * Copyright (C) 2018-2020 Marc3842h, czapek
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

#include "../../utils/bot_utils.hh"
#include "switch_command.hh"

bool shiro::commands::relax(std::deque<std::string>& args, std::shared_ptr<shiro::users::user> user, std::string channel) {
    if (user->status.play_mode == 3) {
        utils::bot::respond("Mania don't have Relax mode.", user, channel, true);
        return true;
    }

    if (user->is_relax) {
        utils::bot::respond("Current mode is already Relax.", user, channel, true);
        return true;
    }

    user->update(true);
    user->refresh_stats();

    utils::bot::respond("Your current mode is Relax!", user, channel, true);

    return true;
}

bool shiro::commands::classic(std::deque<std::string>& args, std::shared_ptr<shiro::users::user> user, std::string channel) {
    if (!user->is_relax) {
        utils::bot::respond("Current mode is already Classic.", user, channel, true);
        return true;
    }

    user->update(false);
    user->refresh_stats();

    utils::bot::respond("Your current mode is Classic!", user, channel, true);

    return true;
}