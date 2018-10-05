/*
 * shiro - High performance, high quality osu!Bancho C++ re-implementation
 * Copyright (C) 2018 Marc3842h, czapek
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

#include "../../../utils/osu_string.hh"
#include "multiplayer_match.hh"

shiro::io::buffer shiro::io::layouts::multiplayer_match::marshal() {
    std::string game_name = utils::osu_string(this->game_name);
    std::string game_password = utils::osu_string(this->game_password, true);
    std::string beatmap_name = utils::osu_string(this->beatmap_name);
    std::string beatmap_checksum = utils::osu_string(this->beatmap_checksum);

    io::buffer buf;

    buf.write<int16_t>(this->match_id);
    buf.write<bool>(this->in_progress);

    buf.write<uint8_t>(this->multi_type);
    buf.write<int32_t>(this->active_mods);

    buf.write_string(game_name);
    buf.write_string(game_password);

    buf.write_string(beatmap_name);
    buf.write<int32_t>(this->beatmap_id);
    buf.write_string(beatmap_checksum);

    for (uint8_t status : this->multi_slot_status) {
        buf.write<uint8_t>(status);
    }

    for (uint8_t team : this->multi_slot_team) {
        buf.write<uint8_t>(team);
    }

    for (size_t i = 0; i < this->multi_slot_status.size(); i++) {
        int32_t status = this->multi_slot_status.at(i) & 0x7c;

        if (status > 0)
            buf.write<int32_t>(this->multi_slot_id[i]);
    }

    buf.write<int32_t>(this->host_id);

    buf.write<uint8_t>(this->game_mode);
    buf.write<uint8_t>(this->multi_win_condition);
    buf.write<uint8_t>(this->multi_team_type);
    buf.write<uint8_t>(this->multi_special_modes);

    int32_t special_mode = this->multi_special_modes & 0x1;

    if (special_mode > 0) {
        for (int32_t mods : this->multi_slot_mods) {
            buf.write<int32_t>(mods);
        }
    }

    buf.write<int32_t>(this->random_seed);

    return buf;
}

void shiro::io::layouts::multiplayer_match::unmarshal(shiro::io::buffer &buffer) {
    this->match_id = buffer.read<int16_t>();
    this->in_progress = buffer.read<bool>();

    this->multi_type = buffer.read<uint8_t>();
    this->active_mods = buffer.read<int32_t>();

    this->game_name = buffer.read_string();
    this->game_password = buffer.read_string();

    this->beatmap_name = buffer.read_string();
    this->beatmap_id = buffer.read<int32_t>();
    this->beatmap_checksum = buffer.read_string();

    for (uint8_t &status : this->multi_slot_status) {
        status = buffer.read<uint8_t>();
    }

    for (uint8_t &team : this->multi_slot_team) {
        team = buffer.read<uint8_t>();
    }

    for (size_t i = 0; i < this->multi_slot_status.size(); i++) {
        int32_t status = this->multi_slot_status[i] & 0x7c;

        this->multi_slot_id.at(i) = status > 0 ? buffer.read<int32_t>() : -1;
    }

    this->host_id = buffer.read<int32_t>();

    this->game_mode = buffer.read<uint8_t>();
    this->multi_win_condition = buffer.read<uint8_t>();
    this->multi_team_type = buffer.read<uint8_t>();
    this->multi_special_modes = buffer.read<uint8_t>();

    int32_t special_mode = this->multi_special_modes & 0x1;

    if (special_mode > 0) {
        for (int32_t &mod : this->multi_slot_mods) {
            mod = buffer.read<int32_t>();
        }
    }

    this->random_seed = buffer.read<int32_t>();
}

int32_t shiro::io::layouts::multiplayer_match::get_size() {
    return this->marshal().get_size();
}