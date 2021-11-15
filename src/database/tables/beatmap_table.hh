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

#ifndef SHIRO_BEATMAP_TABLE_HH
#define SHIRO_BEATMAP_TABLE_HH

#include <sqlpp11/char_sequence.h>
#include <sqlpp11/column_types.h>
#include <sqlpp11/mysql/connection.h>
#include <sqlpp11/table.h>

#include "common_tables.hh"

namespace shiro::tables {

    struct beatmaps_objects {
        object_struct(id, sqlpp::integer);
        object_struct(beatmap_id, sqlpp::integer);
        object_struct(beatmapset_id, sqlpp::integer);
        object_struct(beatmap_md5, sqlpp::varchar);
        object_struct(artist, sqlpp::varchar);
        object_struct(title, sqlpp::varchar);
        object_struct(difficulty_name, sqlpp::varchar);
        object_struct(creator, sqlpp::varchar);
        object_struct(cs, sqlpp::floating_point);
        object_struct(ar, sqlpp::floating_point);
        object_struct(od, sqlpp::floating_point);
        object_struct(hp, sqlpp::floating_point);
        object_struct(mode, sqlpp::integer);
        object_struct(difficulty_std, sqlpp::floating_point);
        object_struct(difficulty_taiko, sqlpp::floating_point);
        object_struct(difficulty_ctb, sqlpp::floating_point);
        object_struct(difficulty_mania, sqlpp::floating_point);
        object_struct(max_combo, sqlpp::integer);
        object_struct(hit_length, sqlpp::integer);
        object_struct(bpm, sqlpp::integer);
        object_struct(count_normal, sqlpp::integer);
        object_struct(count_slider, sqlpp::integer);
        object_struct(count_spinner, sqlpp::integer);
        object_struct(play_count, sqlpp::integer);
        object_struct(pass_count, sqlpp::integer);
        object_struct(ranked_status, sqlpp::integer);
        object_struct(latest_update, sqlpp::bigint);
        object_struct(ranked_status_freezed, sqlpp::boolean);
        object_struct(creating_date, sqlpp::bigint);
    };

    database_table(beatmaps,
        beatmaps_objects::id,
        beatmaps_objects::beatmap_id,
        beatmaps_objects::beatmapset_id,
        beatmaps_objects::beatmap_md5,
        beatmaps_objects::artist,
        beatmaps_objects::title,
        beatmaps_objects::difficulty_name,
        beatmaps_objects::creator,
        beatmaps_objects::cs,
        beatmaps_objects::ar,
        beatmaps_objects::od,
        beatmaps_objects::hp,
        beatmaps_objects::mode,
        beatmaps_objects::difficulty_std,
        beatmaps_objects::difficulty_taiko,
        beatmaps_objects::difficulty_ctb,
        beatmaps_objects::difficulty_mania,
        beatmaps_objects::max_combo,
        beatmaps_objects::hit_length,
        beatmaps_objects::bpm,
        beatmaps_objects::count_normal,
        beatmaps_objects::count_slider,
        beatmaps_objects::count_spinner,
        beatmaps_objects::play_count,
        beatmaps_objects::pass_count,
        beatmaps_objects::ranked_status,
        beatmaps_objects::latest_update,
        beatmaps_objects::ranked_status_freezed,
        beatmaps_objects::creating_date
    );
}

#endif  // SHIRO_BEATMAP_TABLE_HH
