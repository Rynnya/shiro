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

#ifndef SHIRO_PUNISHMENTS_TABLE_HH
#define SHIRO_PUNISHMENTS_TABLE_HH

#include <sqlpp11/char_sequence.h>
#include <sqlpp11/column_types.h>
#include <sqlpp11/mysql/connection.h>
#include <sqlpp11/table.h>

#include "common_tables.hh"

namespace shiro::tables {

    struct punishments_objects {
        object_struct(id, sqlpp::integer);
        object_struct(user_id, sqlpp::integer);
        object_struct(origin_id, sqlpp::integer);
        object_struct(type, sqlpp::tinyint);
        object_struct(time, sqlpp::bigint);
        object_struct(duration, sqlpp::integer);
        object_struct(active, sqlpp::boolean);
        object_struct(reason, sqlpp::varchar);
    };

    database_table(punishments,
        punishments_objects::id,
        punishments_objects::user_id,
        punishments_objects::origin_id,
        punishments_objects::type,
        punishments_objects::time,
        punishments_objects::duration,
        punishments_objects::active,
        punishments_objects::reason
    );

    constexpr punishments punishments_table {};
}

#endif  // SHIRO_PUNISHMENTS_TABLE_HH
