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

#ifndef SHIRO_REPLAY_MANAGER_HH
#define SHIRO_REPLAY_MANAGER_HH

#include <string>

#include "../beatmaps/beatmap.hh"
#include "../scores/score.hh"

namespace shiro::replays {

    void init();

    void save_replay(const scores::score &s, const beatmaps::beatmap &beatmap, std::string replay);

    // osu! requires raw replay in-game, to get full replay use `/api/get_replay` or `get_full_replay(score)`
    std::string get_replay(const scores::score &s);

    std::string get_full_replay(const scores::score &s);

    bool has_replay(const scores::score &s);

}

#endif //SHIRO_REPLAY_MANAGER_HH
