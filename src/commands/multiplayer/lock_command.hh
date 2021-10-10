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

#ifndef SHIRO_LOCK_COMMAND_MP_HH
#define SHIRO_LOCK_COMMAND_MP_HH

#include <deque>
#include <string>

#include "../../users/user.hh"

namespace shiro::commands_mp
{

    bool lock(std::deque<std::string>& args, std::shared_ptr<users::user> user, std::string channel);

}

#endif //SHIRO_LOCK_COMMAND_MP_HH
