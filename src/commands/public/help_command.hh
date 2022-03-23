/*
 * shiro - High performance, high quality osu!Bancho C++ re-implementation
 * Copyright (C) 2018-2020 Marc3842h, czapek
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

#ifndef SHIRO_HELP_COMMAND_HH
#define SHIRO_HELP_COMMAND_HH

#include <deque>
#include <string>
#include <unordered_map>

#include "../../users/user.hh"
#include "../../permissions/permissions.hh"

namespace shiro::commands {

    extern const std::unordered_map<shiro::permissions::perms, std::string> staff_commands;

    bool help(std::deque<std::string>& args, const std::shared_ptr<shiro::users::user>& user, const std::string& channel);

}

#endif //SHIRO_HELP_COMMAND_HH
