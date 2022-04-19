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

#ifndef SHIRO_CHANNEL_TABLE_HH
#define SHIRO_CHANNEL_TABLE_HH

#include <sqlpp11/char_sequence.h>
#include <sqlpp11/column_types.h>
#include <sqlpp11/mysql/connection.h>
#include <sqlpp11/table.h>

#include "common_tables.hh"

namespace shiro::tables {

    struct channel_objects {
        object_struct(id, sqlpp::integer);
        object_struct(name, sqlpp::varchar);
        object_struct(description, sqlpp::varchar);
        object_struct(auto_join, sqlpp::boolean);
        object_struct(hidden, sqlpp::boolean);
        object_struct(read_only, sqlpp::boolean);
        object_struct(permission, sqlpp::bigint);
    };

    database_table(channels,
        channel_objects::id,
        channel_objects::name,
        channel_objects::description,
        channel_objects::auto_join,
        channel_objects::hidden,
        channel_objects::read_only,
        channel_objects::permission
    );

    constexpr channels channels_table {};
}

#endif  // SHIRO_CHANNEL_TABLE_HH
