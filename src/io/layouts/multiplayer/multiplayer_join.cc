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

#include "multiplayer_join.hh"

shiro::io::buffer shiro::io::layouts::multiplayer_join::marshal() {
    io::buffer buf;

    buf.write<int32_t>(this->match_id);
    buf.write<std::string>(this->password);

    return buf;
}

void shiro::io::layouts::multiplayer_join::unmarshal(shiro::io::buffer &buffer) {
    this->match_id = buffer.read<int32_t>();
    this->password = buffer.read<std::string>();
}

int32_t shiro::io::layouts::multiplayer_join::get_size() {
    return static_cast<int32_t>(this->marshal().get_size());
}
