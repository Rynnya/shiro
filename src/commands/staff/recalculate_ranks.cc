/*
 * shiro - High performance, high quality osu!Bancho C++ re-implementation
 * Copyright (C) 2018-2020 Marc3842h, czapek
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

#include <boost/lexical_cast.hpp>

#include "../../permissions/permissions.hh"
#include "../../permissions/role_manager.hh"
#include "../../pp/pp_recalculator.hh"
#include "../../utils/bot_utils.hh"
#include "../../utils/string_utils.hh"
#include "recalculate_ranks.hh"

bool shiro::commands::recalculate(std::deque<std::string> &args, std::shared_ptr<shiro::users::user> user, std::string channel) {
    if (!roles::manager::has_permission(user, permissions::perms::cmd_recalculate)) {
        utils::bot::respond("Permission denied. (" + std::to_string(static_cast<uint64_t>(permissions::perms::cmd_recalculate)) + ")", user, channel, true);
        return false;
    }

    utils::play_mode mode = static_cast<utils::play_mode>(user->stats.play_mode);
    bool is_relax = false;

    if (args.size() >= 2) {
        uint8_t parsed_mode = 0;
        if (!utils::strings::evaluate(args.at(0), parsed_mode)) {
            utils::bot::respond("Unable to parse provided game mode into integer.", std::move(user), std::move(channel), true);
            return false;
        }

        mode = static_cast<utils::play_mode>(parsed_mode);
        is_relax = utils::strings::evaluate<bool>(args.at(1));
    }

    pp::recalculator::begin(mode, is_relax);

    utils::bot::respond("Success! PP recalculating will begin now for all users.", std::move(user), std::move(channel));
    return true;
}
