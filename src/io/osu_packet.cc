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

#include "osu_packet.hh"

shiro::io::osu_packet::osu_packet(buffer &data) {
    this->id = static_cast<packet_id>(data.read<uint16_t>());
    data.read<uint8_t>();

    int data_size = data.read<int32_t>();

    for (int i = 0; i < data_size; i++) {
        this->data.write<uint8_t>(static_cast<char>(data.read<uint8_t>()));
    }
}
