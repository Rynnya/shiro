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

#include "../../../utils/osu_string.hh"
#include "message.hh"

shiro::io::layouts::message::message(std::string sender, int32_t sender_id, std::string content, std::string channel)
        : sender(std::move(sender))
        , sender_id(sender_id)
        , content(std::move(content))
        , channel(std::move(channel)) {
    // Initialized in initializer list
}

shiro::io::buffer shiro::io::layouts::message::marshal() {
    if (this->content.length() < 512) {
        this->content.resize(512);
    }

    std::string sender = utils::osu_string(this->sender);
    std::string content = utils::osu_string(this->content);
    std::string channel = utils::osu_string(this->channel);

    buffer buf;

    buf.write<std::string>(sender);

    buf.write<std::string>(content);
    buf.write<std::string>(channel);

    buf.write<int32_t>(this->sender_id);

    return buf;
}

void shiro::io::layouts::message::unmarshal(shiro::io::buffer &buffer) {
    this->sender = buffer.read<std::string>();

    std::string content = buffer.read<std::string>();

    if (content.length() > 512) {
        content.resize(512);
    }

    this->content = content;
    this->channel = buffer.read<std::string>();

    this->sender_id = buffer.read<int32_t>();
}

int32_t shiro::io::layouts::message::get_size() {
    return static_cast<int32_t>(this->marshal().get_size());
}
