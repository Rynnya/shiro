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

#ifndef SHIRO_SCORE_TABLE_HH
#define SHIRO_SCORE_TABLE_HH

#include <sqlpp11/char_sequence.h>
#include <sqlpp11/column_types.h>
#include <sqlpp11/mysql/connection.h>
#include <sqlpp11/table.h>

#include "common_tables.hh"

namespace shiro::tables {

    struct scores_objects {
        object_struct(id, sqlpp::integer);
        object_struct(hash, sqlpp::varchar);
        object_struct(beatmap_md5, sqlpp::varchar);
        object_struct(user_id, sqlpp::integer);
        object_struct(ranking, sqlpp::varchar);
        object_struct(score, sqlpp::bigint);
        object_struct(max_combo, sqlpp::integer);
        object_struct(full_combo, sqlpp::boolean);
        object_struct(mods, sqlpp::integer);
        object_struct(count_300, sqlpp::integer);
        object_struct(count_100, sqlpp::integer);
        object_struct(count_50, sqlpp::integer);
        object_struct(count_katus, sqlpp::integer);
        object_struct(count_gekis, sqlpp::integer);
        object_struct(count_misses, sqlpp::integer);
        object_struct(time, sqlpp::integer);
        object_struct(play_mode, sqlpp::tinyint);
        object_struct(completed, sqlpp::boolean);
        object_struct(accuracy, sqlpp::floating_point);
        object_struct(pp, sqlpp::floating_point);
        object_struct(times_watched, sqlpp::integer);
        object_struct(is_relax, sqlpp::boolean);
    };

    database_table(scores,
            scores_objects::id,
            scores_objects::hash,
            scores_objects::beatmap_md5,
            scores_objects::user_id,
            scores_objects::ranking,
            scores_objects::score,
            scores_objects::max_combo,
            scores_objects::full_combo,
            scores_objects::mods,
            scores_objects::count_300,
            scores_objects::count_100,
            scores_objects::count_50,
            scores_objects::count_katus,
            scores_objects::count_gekis,
            scores_objects::count_misses,
            scores_objects::time,
            scores_objects::play_mode,
            scores_objects::completed,
            scores_objects::accuracy,
            scores_objects::pp,
            scores_objects::times_watched,
            scores_objects::is_relax
    );

    namespace migrations::scores {

        inline void create(sqlpp::mysql::connection &db) {
            //db.execute(
            //        "CREATE TABLE IF NOT EXISTS `scores` "
            //        "(id INT PRIMARY KEY NOT NULL AUTO_INCREMENT, "
            //        "beatmap_md5 VARCHAR(35) NOT NULL, hash VARCHAR(35) NOT NULL, user_id INT NOT NULL, ranking VARCHAR(2) NOT NULL, "
            //        "score BIGINT NOT NULL, max_combo INT NOT NULL, fc BOOLEAN NOT NULL, mods INT NOT NULL, "
            //        "300_count INT NOT NULL, 100_count INT NOT NULL, 50_count INT NOT NULL, "
            //        "katus_count INT NOT NULL, gekis_count INT NOT NULL, miss_count INT NOT NULL, "
            //        "time INT NOT NULL, play_mode TINYINT NOT NULL, passed BOOLEAN NOT NULL, "
            //        "accuracy FLOAT NOT NULL, pp FLOAT NOT NULL, times_watched INT NOT NULL DEFAULT 0);"
            //);
        }

    }

}

#endif  // SHIRO_SCORE_TABLE_HH
