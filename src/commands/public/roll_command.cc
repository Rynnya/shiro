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

#include <random>

#include "../../utils/bot_utils.hh"
#include "../../utils/string_utils.hh"
#include "roll_command.hh"

static std::random_device random_device;
static std::mt19937 engine(random_device());

bool shiro::commands::roll(std::deque<std::string> &args, std::shared_ptr<shiro::users::user> user, std::string channel)
{
    int32_t max = 100;

    if (!args.empty()) // On error max won't changes
        std::ignore = utils::strings::safe_int(args.at(0), max);

    if (max < 0)
        max = 1;

    std::uniform_int_distribution<> distribution(0, max);
    int roll = distribution(engine);
    std::string extension = roll != 1 ? "s" : "";

    utils::bot::respond(user->presence.username + " rolls " + std::to_string(roll) + " point" + extension + ".", user, channel);
    return true;
}
