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

#ifndef SHIRO_USER_TABLE_HH
#define SHIRO_USER_TABLE_HH

#include <sqlpp11/char_sequence.h>
#include <sqlpp11/column_types.h>
#include <sqlpp11/mysql/connection.h>
#include <sqlpp11/table.h>

#include "common_tables.hh"

namespace shiro::tables {

	struct users_objects {
		object_struct(id, sqlpp::integer);
		object_struct(username, sqlpp::varchar);
		object_struct(safe_username, sqlpp::varchar);
		object_struct(country, sqlpp::varchar);
		object_struct(password_md5, sqlpp::varchar);
		object_struct(salt, sqlpp::varchar);
		object_struct(email, sqlpp::varchar);
		object_struct(ip, sqlpp::varchar);
		object_struct(registration_date, sqlpp::integer);
		object_struct(latest_activity, sqlpp::integer);
		object_struct(followers, sqlpp::integer);
		object_struct(roles, sqlpp::bigint_unsigned);
		object_struct(userpage, sqlpp::text);
		object_struct(is_relax, sqlpp::boolean);
	};

	struct users_stats_objects
	{
		object_struct(id, sqlpp::integer);
		object_struct(rank_std, sqlpp::integer);
		object_struct(rank_taiko, sqlpp::integer);
		object_struct(rank_ctb, sqlpp::integer);
		object_struct(rank_mania, sqlpp::integer);
		object_struct(ranked_score_std, sqlpp::bigint);
		object_struct(ranked_score_taiko, sqlpp::bigint);
		object_struct(ranked_score_ctb, sqlpp::bigint);
		object_struct(ranked_score_mania, sqlpp::bigint);
		object_struct(total_score_std, sqlpp::bigint);
		object_struct(total_score_taiko, sqlpp::bigint);
		object_struct(total_score_ctb, sqlpp::bigint);
		object_struct(total_score_mania, sqlpp::bigint);
		object_struct(play_count_std, sqlpp::integer);
		object_struct(play_count_taiko, sqlpp::integer);
		object_struct(play_count_ctb, sqlpp::integer);
		object_struct(play_count_mania, sqlpp::integer);
		object_struct(replays_watched_std, sqlpp::integer);
		object_struct(replays_watched_taiko, sqlpp::integer);
		object_struct(replays_watched_ctb, sqlpp::integer);
		object_struct(replays_watched_mania, sqlpp::integer);
		object_struct(total_hits_std, sqlpp::integer);
		object_struct(total_hits_taiko, sqlpp::integer);
		object_struct(total_hits_ctb, sqlpp::integer);
		object_struct(total_hits_mania, sqlpp::integer);
		object_struct(max_combo_std, sqlpp::integer);
		object_struct(max_combo_taiko, sqlpp::integer);
		object_struct(max_combo_ctb, sqlpp::integer);
		object_struct(max_combo_mania, sqlpp::integer);
		object_struct(play_time_std, sqlpp::integer);
		object_struct(play_time_taiko, sqlpp::integer);
		object_struct(play_time_ctb, sqlpp::integer);
		object_struct(play_time_mania, sqlpp::integer);
		object_struct(avg_accuracy_std, sqlpp::floating_point);
		object_struct(avg_accuracy_taiko, sqlpp::floating_point);
		object_struct(avg_accuracy_ctb, sqlpp::floating_point);
		object_struct(avg_accuracy_mania, sqlpp::floating_point);
		object_struct(pp_std, sqlpp::integer);
		object_struct(pp_taiko, sqlpp::integer);
		object_struct(pp_ctb, sqlpp::integer);
		object_struct(pp_mania, sqlpp::integer);
		object_struct(count_A_std, sqlpp::integer);
		object_struct(count_S_std, sqlpp::integer);
		object_struct(count_X_std, sqlpp::integer);
		object_struct(count_SH_std, sqlpp::integer);
		object_struct(count_XH_std, sqlpp::integer);
		object_struct(count_A_taiko, sqlpp::integer);
		object_struct(count_S_taiko, sqlpp::integer);
		object_struct(count_X_taiko, sqlpp::integer);
		object_struct(count_SH_taiko, sqlpp::integer);
		object_struct(count_XH_taiko, sqlpp::integer);
		object_struct(count_A_ctb, sqlpp::integer);
		object_struct(count_S_ctb, sqlpp::integer);
		object_struct(count_X_ctb, sqlpp::integer);
		object_struct(count_SH_ctb, sqlpp::integer);
		object_struct(count_XH_ctb, sqlpp::integer);
		object_struct(count_A_mania, sqlpp::integer);
		object_struct(count_S_mania, sqlpp::integer);
		object_struct(count_X_mania, sqlpp::integer);
		object_struct(count_SH_mania, sqlpp::integer);
		object_struct(count_XH_mania, sqlpp::integer);
	};

	database_table(users,
		users_objects::id,
		users_objects::username,
		users_objects::safe_username,
		users_objects::country,
		users_objects::password_md5,
		users_objects::salt,
		users_objects::email,
		users_objects::ip,
		users_objects::registration_date,
		users_objects::latest_activity,
		users_objects::followers,
		users_objects::roles,
		users_objects::userpage,
		users_objects::is_relax
	);

	database_table(users_stats,
		users_stats_objects::id,
		users_stats_objects::rank_std,
		users_stats_objects::rank_taiko,
		users_stats_objects::rank_ctb,
		users_stats_objects::rank_mania,
		users_stats_objects::ranked_score_std,
		users_stats_objects::ranked_score_taiko,
		users_stats_objects::ranked_score_ctb,
		users_stats_objects::ranked_score_mania,
		users_stats_objects::total_score_std,
		users_stats_objects::total_score_taiko,
		users_stats_objects::total_score_ctb,
		users_stats_objects::total_score_mania,
		users_stats_objects::play_count_std,
		users_stats_objects::play_count_taiko,
		users_stats_objects::play_count_ctb,
		users_stats_objects::play_count_mania,
		users_stats_objects::replays_watched_std,
		users_stats_objects::replays_watched_taiko,
		users_stats_objects::replays_watched_ctb,
		users_stats_objects::replays_watched_mania,
		users_stats_objects::total_hits_std,
		users_stats_objects::total_hits_taiko,
		users_stats_objects::total_hits_ctb,
		users_stats_objects::total_hits_mania,
		users_stats_objects::max_combo_std,
		users_stats_objects::max_combo_taiko,
		users_stats_objects::max_combo_ctb,
		users_stats_objects::max_combo_mania,
		users_stats_objects::play_time_std,
		users_stats_objects::play_time_taiko,
		users_stats_objects::play_time_ctb,
		users_stats_objects::play_time_mania,
		users_stats_objects::avg_accuracy_std,
		users_stats_objects::avg_accuracy_taiko,
		users_stats_objects::avg_accuracy_ctb,
		users_stats_objects::avg_accuracy_mania,
		users_stats_objects::pp_std,
		users_stats_objects::pp_taiko,
		users_stats_objects::pp_ctb,
		users_stats_objects::pp_mania,
		users_stats_objects::count_A_std,
		users_stats_objects::count_S_std,
		users_stats_objects::count_SH_std,
		users_stats_objects::count_X_std,
		users_stats_objects::count_XH_std,
		users_stats_objects::count_A_taiko,
		users_stats_objects::count_S_taiko,
		users_stats_objects::count_SH_taiko,
		users_stats_objects::count_X_taiko,
		users_stats_objects::count_XH_taiko,
		users_stats_objects::count_A_ctb,
		users_stats_objects::count_S_ctb,
		users_stats_objects::count_SH_ctb,
		users_stats_objects::count_X_ctb,
		users_stats_objects::count_XH_ctb,
		users_stats_objects::count_A_mania,
		users_stats_objects::count_S_mania,
		users_stats_objects::count_SH_mania,
		users_stats_objects::count_X_mania,
		users_stats_objects::count_XH_mania
	);

	database_table(users_stats_relax,
		users_stats_objects::id,
		users_stats_objects::rank_std,
		users_stats_objects::rank_taiko,
		users_stats_objects::rank_ctb,
		users_stats_objects::ranked_score_std,
		users_stats_objects::ranked_score_taiko,
		users_stats_objects::ranked_score_ctb,
		users_stats_objects::total_score_std,
		users_stats_objects::total_score_taiko,
		users_stats_objects::total_score_ctb,
		users_stats_objects::play_count_std,
		users_stats_objects::play_count_taiko,
		users_stats_objects::play_count_ctb,
		users_stats_objects::replays_watched_std,
		users_stats_objects::replays_watched_taiko,
		users_stats_objects::replays_watched_ctb,
		users_stats_objects::total_hits_std,
		users_stats_objects::total_hits_taiko,
		users_stats_objects::total_hits_ctb,
		users_stats_objects::max_combo_std,
		users_stats_objects::max_combo_taiko,
		users_stats_objects::max_combo_ctb,
		users_stats_objects::play_time_std,
		users_stats_objects::play_time_taiko,
		users_stats_objects::play_time_ctb,
		users_stats_objects::avg_accuracy_std,
		users_stats_objects::avg_accuracy_taiko,
		users_stats_objects::avg_accuracy_ctb,
		users_stats_objects::pp_std,
		users_stats_objects::pp_taiko,
		users_stats_objects::pp_ctb,
		users_stats_objects::count_A_std,
		users_stats_objects::count_S_std,
		users_stats_objects::count_SH_std,
		users_stats_objects::count_X_std,
		users_stats_objects::count_XH_std,
		users_stats_objects::count_A_taiko,
		users_stats_objects::count_S_taiko,
		users_stats_objects::count_SH_taiko,
		users_stats_objects::count_X_taiko,
		users_stats_objects::count_XH_taiko,
		users_stats_objects::count_A_ctb,
		users_stats_objects::count_S_ctb,
		users_stats_objects::count_SH_ctb,
		users_stats_objects::count_X_ctb,
		users_stats_objects::count_XH_ctb
	);

	namespace migrations::users {

		inline void create(sqlpp::mysql::connection& db) {
			//db.execute(
			//        "CREATE TABLE IF NOT EXISTS `users` "
			//        "(id INT PRIMARY KEY NOT NULL AUTO_INCREMENT, "
			//        "username VARCHAR(32) NOT NULL, safe_username VARCHAR(32) NOT NULL, "
			//        "password VARCHAR(128) NOT NULL, salt VARCHAR(64) NOT NULL,"
			//        "email VARCHAR(100) NOT NULL, ip VARCHAR(39) NOT NULL, registration_date INT NOT NULL, last_seen INT NOT NULL, "
			//        "followers INT NOT NULL, roles INT UNSIGNED NOT NULL, user_page TEXT NOT NULL, "
			//        "pp_std FLOAT NOT NULL DEFAULT 0, pp_taiko FLOAT NOT NULL DEFAULT 0, pp_ctb FLOAT NOT NULL DEFAULT 0, "
			//        "pp_mania FLOAT NOT NULL DEFAULT 0, rank_std INT NOT NULL DEFAULT 0, rank_taiko INT NOT NULL DEFAULT 0, "
			//        "rank_ctb INT NOT NULL DEFAULT 0, rank_mania INT NOT NULL DEFAULT 0, score_std INT NOT NULL DEFAULT 0, "
			//        "score_taiko INT NOT NULL DEFAULT 0, score_ctb INT NOT NULL DEFAULT 0, score_mania INT NOT NULL DEFAULT 0, "
			//        "ranked_score_std INT NOT NULL DEFAULT 0, ranked_score_taiko INT NOT NULL DEFAULT 0, "
			//        "ranked_score_ctb INT NOT NULL DEFAULT 0, ranked_score_mania INT NOT NULL DEFAULT 0, "
			//        "accuracy_std FLOAT NOT NULL DEFAULT 0, accuracy_taiko FLOAT NOT NULL DEFAULT 0, accuracy_ctb FLOAT NOT NULL DEFAULT 0, "
			//        "accuracy_mania FLOAT NOT NULL DEFAULT 0, play_count_std INT NOT NULL DEFAULT 0, play_count_taiko INT NOT NULL DEFAULT 0, "
			//        "play_count_ctb INT NOT NULL DEFAULT 0, play_count_mania INT NOT NULL DEFAULT 0, max_combo_std INT NOT NULL DEFAULT 0, "
			//        "max_combo_taiko INT NOT NULL DEFAULT 0, max_combo_ctb INT NOT NULL DEFAULT 0, max_combo_mania INT NOT NULL DEFAULT 0, "
			//        "country VARCHAR(2) NOT NULL);"
			//);
		}

	}

}

#endif  // SHIRO_USER_TABLE_HH
