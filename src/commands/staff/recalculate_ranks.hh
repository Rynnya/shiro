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

#ifndef SHIRO_RECALCULATE_RANKS_HH
#define SHIRO_RECALCULATE_RANKS_HH

#include <deque>
#include <string>

#include "../../users/user.hh"

namespace shiro::commands {

    bool recalculate(std::deque<std::string>& args, const std::shared_ptr<shiro::users::user>& user, const std::string& channel);

}

#endif //SHIRO_RECALCULATE_RANKS_HH
