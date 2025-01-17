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

#include <cstring>
#include <utility>

#include "../database/tables/channel_table.hh"
#include "../permissions/permissions.hh"
#include "../thirdparty/naga.hh"
#include "../shiro.hh"
#include "channel_manager.hh"

std::unordered_map<shiro::io::layouts::channel, std::vector<std::shared_ptr<shiro::users::user>>> shiro::channels::manager::channels;
std::shared_timed_mutex shiro::channels::manager::mutex;
std::vector<std::pair<uint32_t, shiro::io::layouts::channel>> shiro::channels::manager::auto_join_channels;

void shiro::channels::manager::init() {
    if (!channels.empty()) {
        channels.clear();
    }

    auto db = shiro::database::instance->pop();

    insert_if_not_exists("#announce", "", true, false, true, 0);
    insert_if_not_exists("#lobby", "", false, true, false, 0);
    insert_if_not_exists("#console", "", true, false, true, static_cast<int64_t>(permissions::perms::console_chat));

    auto result = db(select(all_of(tables::channels_table)).from(tables::channels_table).unconditionally());

    if (result.empty()) {
        return;
    }

    // Disallow other threads from both writing and reading
    std::unique_lock<std::shared_timed_mutex> lock(mutex);

    for (const auto &row : result) {
        std::string name = row.name;

        if (name.at(0) != '#') {
            LOG_F(WARNING, "Channel name of channel id {0} doesn't start with #, fixing ({1} -> #{1}).", row.id.value(), name);
            name.insert(0, "#");

            db(update(tables::channels_table).set(tables::channels_table.name = name).where(tables::channels_table.id == row.id));
        }

        io::layouts::channel channel(row.id, row.auto_join, row.hidden, name, row.description, 0, row.read_only, row.permission);
        channels.insert({ channel, { } });

        if (!channel.auto_join) {
            continue;
        }

        auto_join_channels.emplace_back(channel.id, channel);
    }
}

void shiro::channels::manager::write_channels(shiro::io::osu_writer &buffer, std::shared_ptr<shiro::users::user> user) {
    // Disallow other threads from writing (but not from reading)
    std::shared_lock<std::shared_timed_mutex> shared_lock(mutex);

    for (const auto &[channel, users] : channels) {
        if (channel.hidden || !has_permissions(user, channel.permission)) {
            continue;
        }

        shiro::io::layouts::channel channel_layout;
        channel_layout.id = channel.id;
        channel_layout.auto_join = channel.auto_join;
        channel_layout.name = channel.name;
        channel_layout.description = channel.description;
        channel_layout.user_count = users.size();

        buffer.channel_available(channel_layout);
    }
}

void shiro::channels::manager::auto_join(shiro::io::osu_writer &buffer, std::shared_ptr<shiro::users::user> user) {
    for (const auto &[channel_id, channel] : auto_join_channels) {
        if (!has_permissions(user, channel.permission)) {
            continue;
        }

        if (!join_channel(channel_id, user)) {
            continue;
        }

        buffer.channel_join(channel.name);
    }
}

bool shiro::channels::manager::join_channel(uint32_t channel_id, std::shared_ptr<shiro::users::user> user) {
    if (in_channel(channel_id, user)) {
        return leave_channel(channel_id, user);
    }

    // Disallow other threads from both writing and reading
    std::unique_lock<std::shared_timed_mutex> lock(mutex);

    for (auto &[channel, users] : channels) {
        if (channel.id != channel_id || !has_permissions(user, channel.permission)) {
            continue;
        }

        users.emplace_back(user);
        return true;
    }

    return false;
}

bool shiro::channels::manager::leave_channel(uint32_t channel_id, std::shared_ptr<shiro::users::user> user) {
    if (!in_channel(channel_id, user)) {
        return true;
    }

    // Disallow other threads from both writing and reading
    std::unique_lock<std::shared_timed_mutex> lock(mutex);

    for (auto &[channel, users] : channels) {
        if (channel.id != channel_id || !has_permissions(user, channel.permission)) {
            continue;
        }

        auto iterator = std::find(users.begin(), users.end(), user);

        if (iterator == users.end()) {
            return false;
        }

        users.erase(iterator);
        return true;
    }

    return false;
}

bool shiro::channels::manager::in_channel(uint32_t channel_id, std::shared_ptr<shiro::users::user> user) {
    // Disallow other threads from writing (but not from reading)
    std::shared_lock<std::shared_timed_mutex> lock(mutex);

    for (const auto &[channel, users] : channels) {
        if (channel.id != channel_id) {
            continue;
        }

        return std::find(users.begin(), users.end(), user) != users.end();
    }

    return false;
}

std::vector<std::shared_ptr<shiro::users::user>> shiro::channels::manager::get_users_in_channel(const std::string &channel_name) {
    // Disallow other threads from writing (but not from reading)
    std::shared_lock<std::shared_timed_mutex> lock(mutex);

    for (const auto &[channel, users] : channels) {
        if (channel.name != channel_name) {
            continue;
        }

        return users;
    }

    return {};
}

uint32_t shiro::channels::manager::get_channel_id(const std::string &channel_name) {
    // Disallow other threads from writing (but not from reading)
    std::shared_lock<std::shared_timed_mutex> lock(mutex);

    for (const auto &[channel, _] : channels) {
        if (channel.name != channel_name) {
            continue;
        }

        return channel.id;
    }

    return 0;
}

void shiro::channels::manager::insert_if_not_exists(std::string name, std::string description, bool auto_join, bool hidden, bool read_only, int64_t permission) {
    auto db = shiro::database::instance->pop();

    auto result = db(select(all_of(tables::channels_table)).from(tables::channels_table).where(tables::channels_table.name == name).limit(1u));

    if (!result.empty()) {
        return;
    }

    db(insert_into(tables::channels_table).set(
        tables::channels_table.name = name,
        tables::channels_table.description = std::move(description),
        tables::channels_table.auto_join = auto_join,
        tables::channels_table.hidden = hidden,
        tables::channels_table.read_only = read_only,
        tables::channels_table.permission = permission
    ));
}

bool shiro::channels::manager::has_permissions(std::shared_ptr<shiro::users::user> user, int64_t perms) {
    if (user == nullptr) {
        return false;
    }

    if (perms <= 0) {
        return true;
    }

    return (user->permissions & perms) == perms; // This will also satisfy a array of permissions if required
}

bool shiro::channels::manager::is_read_only(uint32_t channel_id) {
    // Disallow other threads from writing (but not from reading)
    std::shared_lock<std::shared_timed_mutex> lock(mutex);

    for (const auto &[channel, users] : channels) {
        if (channel.id != channel_id) {
            continue;
        }

        return channel.read_only;
    }

    return true;
}

bool shiro::channels::manager::is_read_only(std::string channel_name) {
    // Disallow other threads from writing (but not from reading)
    std::shared_lock<std::shared_timed_mutex> lock(mutex);

    for (const auto &[channel, users] : channels) {
        if (channel.name != channel_name) {
            continue;
        }

        return channel.read_only;
    }

    return false;
}
