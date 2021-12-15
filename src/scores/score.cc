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

#include <sstream>
#include <memory>

#include "../replays/replay_manager.hh"
#include "../users/user.hh"
#include "../users/user_manager.hh"
#include "score.hh"
#include "score_helper.hh"

std::string shiro::scores::score::to_string(std::vector<score> &scores, bool is_relax) {
    std::stringstream stream;

    std::string username = users::manager::get_username_by_id(this->user_id);
    if (username.empty()) {
        return "";
    }

    stream << this->id << "|";
    stream << username << "|";
    stream << (is_relax && this->pp > 0 ? static_cast<int32_t>(this->pp) : this->total_score) << "|";
    stream << this->max_combo << "|";
    stream << this->count_50 << "|";
    stream << this->count_100 << "|";
    stream << this->count_300 << "|";
    stream << this->count_misses << "|";
    stream << this->count_katus << "|";
    stream << this->count_gekis << "|";
    stream << (this->fc ? "True" : "False") << "|";
    stream << this->mods << "|";
    stream << this->user_id << "|";
    stream << helper::get_scoreboard_position(*this, scores) << "|";
    stream << this->time << "|";
    stream << (replays::has_replay(*this) ? "1" : "0") << std::endl;

    return stream.str();
}
