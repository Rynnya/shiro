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

#include <cstring>

#include "../config/ipc_file.hh"
#include "../database/extensions/multirow_insert.hh"
#include "../database/tables/achievements_table.hh"
#include "../database/tables/relationship_table.hh"
#include "../database/tables/user_table.hh"
#include "../permissions/permissions.hh"
#include "../thirdparty/naga.hh"
#include "../utils/crypto.hh"
#include "../utils/play_mode.hh"
#include "../shiro.hh"
#include "user.hh"
#include "user_punishments.hh"

shiro::users::user::user(int32_t user_id) : user_id(user_id) {
    // Initialized in initializer list
}

shiro::users::user::user(const std::string &username) {
    this->presence.username = username;
}

bool shiro::users::user::init() {
    auto db = shiro::database::instance->pop();

    auto user_result = db(select(all_of(tables::users_table))
        .from(tables::users_table)
        .where(tables::users_table.id == this->user_id or tables::users_table.username == this->presence.username)
        .limit(1u));

    if (user_result.empty()) {
        return false;
    }

    const auto &row = user_result.front();

    this->user_id = row.id;
    this->country = row.country;
    this->presence.user_id = this->user_id;
    this->stats.user_id = this->user_id;
    this->presence.username = row.username;
    this->password = row.password_hash;
    this->salt = row.salt;
    this->permissions = row.permissions;
    this->presence.permissions = shiro::permissions::get_chat_color(this->permissions);
    this->is_relax = row.is_relax;

    auto relationship_result = db(select(tables::relationships_table.target)
        .from(tables::relationships_table)
        .where(tables::relationships_table.origin == this->user_id and tables::relationships_table.blocked == false));

    for (const auto& row : relationship_result) {
        this->friends.emplace_back(row.target);
    }

    auto achievements_result = db(select(tables::achievements_table.achievement_id)
        .from(tables::achievements_table)
        .where(tables::achievements_table.user_id == this->user_id));

    for (const auto& row : achievements_result) {
        this->stats.acquired_achievements.emplace_back(row.achievement_id, true);
    }

    this->hidden = users::punishments::is_restricted(this->user_id);
    this->preferences = user_preferences { this->user_id };

    if (is_relax) {
        auto stats_result = db(sqlpp::select(
            tables::users_relax_table.pp_std,
            tables::users_relax_table.max_combo_std,
            tables::users_relax_table.avg_accuracy_std,
            tables::users_relax_table.total_score_std,
            tables::users_relax_table.ranked_score_std,
            tables::users_relax_table.play_count_std,
            tables::users_relax_table.rank_std,
            tables::users_relax_table.play_time_std
        ).from(tables::users_relax_table).where(tables::users_relax_table.id == this->user_id));

        const auto& row_stats = stats_result.front();

        this->stats.pp = row_stats.pp_std;
        this->stats.max_combo = row_stats.max_combo_std;
        this->stats.accuracy = row_stats.avg_accuracy_std;
        this->stats.total_score = row_stats.total_score_std;
        this->stats.ranked_score = row_stats.ranked_score_std;
        this->stats.play_count = row_stats.play_count_std;
        this->stats.rank = row_stats.rank_std;
        this->presence.rank = row_stats.rank_std;
        this->stats.play_time = row_stats.play_time_std;

        return true;
    }

    auto stats_result = db(sqlpp::select(
        tables::users_stats_table.pp_std,
        tables::users_stats_table.max_combo_std,
        tables::users_stats_table.avg_accuracy_std,
        tables::users_stats_table.total_score_std,
        tables::users_stats_table.ranked_score_std,
        tables::users_stats_table.play_count_std,
        tables::users_stats_table.rank_std,
        tables::users_stats_table.play_time_std
    ).from(tables::users_stats_table).where(tables::users_stats_table.id == this->user_id));

    const auto& row_stats = stats_result.front();

    this->stats.pp = row_stats.pp_std;
    this->stats.max_combo = row_stats.max_combo_std;
    this->stats.accuracy = row_stats.avg_accuracy_std;
    this->stats.total_score = row_stats.total_score_std;
    this->stats.ranked_score = row_stats.ranked_score_std;
    this->stats.play_count = row_stats.play_count_std;
    this->stats.rank = row_stats.rank_std;
    this->presence.rank = row_stats.rank_std;
    this->stats.play_time = row_stats.play_time_std;

    return true;
}

void shiro::users::user::update(bool is_relax) {
    auto db = shiro::database::instance->pop();

    db(sqlpp::update(tables::users_table).set(tables::users_table.is_relax = is_relax).where(tables::users_table.id == this->user_id));

    if (is_relax) {
        auto result = db(select(all_of(tables::users_relax_table))
            .from(tables::users_relax_table)
            .where(tables::users_relax_table.id == this->user_id)
            .limit(1u));

        if (result.empty()) {
            return;
        }

        const auto& row = result.front();

        switch (static_cast<utils::play_mode>(this->status.play_mode)) {
            case utils::play_mode::standard: {
                this->stats.pp = row.pp_std;
                this->stats.total_score = row.total_score_std;
                this->stats.ranked_score = row.ranked_score_std;
                this->stats.play_count = row.play_count_std;
                this->stats.rank = row.rank_std;
                this->presence.rank = row.rank_std;
                this->stats.max_combo = row.max_combo_std;
                this->stats.accuracy = row.avg_accuracy_std;
                this->stats.total_hits = row.total_hits_std;
                this->stats.play_time = row.play_time_std;
                this->stats.count_A = row.count_A_std;
                this->stats.count_S = row.count_S_std;
                this->stats.count_X = row.count_X_std;
                this->stats.count_SH = row.count_SH_std;
                this->stats.count_XH = row.count_XH_std;
                break;
            }
            case utils::play_mode::taiko: {
                this->stats.pp = row.pp_taiko;
                this->stats.total_score = row.total_score_taiko;
                this->stats.ranked_score = row.ranked_score_taiko;
                this->stats.play_count = row.play_count_taiko;
                this->stats.rank = row.rank_taiko;
                this->presence.rank = row.rank_taiko;
                this->stats.max_combo = row.max_combo_taiko;
                this->stats.accuracy = row.avg_accuracy_taiko;
                this->stats.total_hits = row.total_hits_taiko;
                this->stats.play_time = row.play_time_taiko;
                this->stats.count_A = row.count_A_taiko;
                this->stats.count_S = row.count_S_taiko;
                this->stats.count_X = row.count_X_taiko;
                this->stats.count_SH = row.count_SH_taiko;
                this->stats.count_XH = row.count_XH_taiko;
                break;
            }
            case utils::play_mode::fruits: {
                this->stats.pp = row.pp_ctb;
                this->stats.total_score = row.total_score_ctb;
                this->stats.ranked_score = row.ranked_score_ctb;
                this->stats.play_count = row.play_count_ctb;
                this->stats.rank = row.rank_ctb;
                this->presence.rank = row.rank_ctb;
                this->stats.max_combo = row.max_combo_ctb;
                this->stats.accuracy = row.avg_accuracy_ctb;
                this->stats.total_hits = row.total_hits_ctb;
                this->stats.play_time = row.play_time_ctb;
                this->stats.count_A = row.count_A_ctb;
                this->stats.count_S = row.count_S_ctb;
                this->stats.count_X = row.count_X_ctb;
                this->stats.count_SH = row.count_SH_ctb;
                this->stats.count_XH = row.count_XH_ctb;
                break;
            }
            case utils::play_mode::mania: {
                break;
            }
        }

        this->is_relax = true;
        return;
    }

    auto result = db(select(all_of(tables::users_stats_table))
        .from(tables::users_stats_table)
        .where(tables::users_stats_table.id == this->user_id)
        .limit(1u));

    if (result.empty()) {
        return;
    }

    const auto& row = result.front();

    switch (static_cast<utils::play_mode>(this->status.play_mode)) {
        case utils::play_mode::standard: {
            this->stats.pp = row.pp_std;
            this->stats.total_score = row.total_score_std;
            this->stats.ranked_score = row.ranked_score_std;
            this->stats.play_count = row.play_count_std;
            this->stats.rank = row.rank_std;
            this->presence.rank = row.rank_std;
            this->stats.max_combo = row.max_combo_std;
            this->stats.accuracy = row.avg_accuracy_std;
            this->stats.total_hits = row.total_hits_std;
            this->stats.play_time = row.play_time_std;
            this->stats.count_A = row.count_A_std;
            this->stats.count_S = row.count_S_std;
            this->stats.count_X = row.count_X_std;
            this->stats.count_SH = row.count_SH_std;
            this->stats.count_XH = row.count_XH_std;
            break;
        }
        case utils::play_mode::taiko: {
            this->stats.pp = row.pp_taiko;
            this->stats.total_score = row.total_score_taiko;
            this->stats.ranked_score = row.ranked_score_taiko;
            this->stats.play_count = row.play_count_taiko;
            this->stats.rank = row.rank_taiko;
            this->presence.rank = row.rank_taiko;
            this->stats.max_combo = row.max_combo_taiko;
            this->stats.accuracy = row.avg_accuracy_taiko;
            this->stats.total_hits = row.total_hits_taiko;
            this->stats.play_time = row.play_time_taiko;
            this->stats.count_A = row.count_A_taiko;
            this->stats.count_S = row.count_S_taiko;
            this->stats.count_X = row.count_X_taiko;
            this->stats.count_SH = row.count_SH_taiko;
            this->stats.count_XH = row.count_XH_taiko;
            break;
        }
        case utils::play_mode::fruits: {
            this->stats.pp = row.pp_ctb;
            this->stats.total_score = row.total_score_ctb;
            this->stats.ranked_score = row.ranked_score_ctb;
            this->stats.play_count = row.play_count_ctb;
            this->stats.rank = row.rank_ctb;
            this->presence.rank = row.rank_ctb;
            this->stats.max_combo = row.max_combo_ctb;
            this->stats.accuracy = row.avg_accuracy_ctb;
            this->stats.total_hits = row.total_hits_ctb;
            this->stats.play_time = row.play_time_ctb;
            this->stats.count_A = row.count_A_ctb;
            this->stats.count_S = row.count_S_ctb;
            this->stats.count_X = row.count_X_ctb;
            this->stats.count_SH = row.count_SH_ctb;
            this->stats.count_XH = row.count_XH_ctb;
            break;
        }
        case utils::play_mode::mania: {
            this->stats.pp = row.pp_mania;
            this->stats.total_score = row.total_score_mania;
            this->stats.ranked_score = row.ranked_score_mania;
            this->stats.play_count = row.play_count_mania;
            this->stats.rank = row.rank_mania;
            this->presence.rank = row.rank_mania;
            this->stats.max_combo = row.max_combo_mania;
            this->stats.accuracy = row.avg_accuracy_mania;
            this->stats.total_hits = row.total_hits_mania;
            this->stats.play_time = row.play_time_mania;
            this->stats.count_A = row.count_A_mania;
            this->stats.count_S = row.count_S_mania;
            this->stats.count_X = row.count_X_mania;
            this->stats.count_SH = row.count_SH_mania;
            this->stats.count_XH = row.count_XH_mania;
            break;
        }
    }

    this->is_relax = false;
}

void shiro::users::user::save_stats(bool to_relax) {
    auto db = shiro::database::instance->pop();
    auto insert_statement = sqlpp::multirow_insert(tables::achievements_table)
        .with_columns(tables::achievements_table.achievement_id, tables::achievements_table.user_id);

    for (size_t i = 0; i < this->stats.acquired_achievements.size(); i++) {
        auto& [id, already_in_db] = this->stats.acquired_achievements[i];

        if (already_in_db) {
            continue;
        }

        insert_statement.set(id, this->user_id);
        already_in_db = true;
    }

    if (insert_statement.size() > 0) {
        db(sqlpp::custom_query(sqlpp::insert_into(tables::achievements_table), insert_statement));
    }
    
    if (to_relax) {
        switch (static_cast<utils::play_mode>(this->stats.play_mode)) {
            case utils::play_mode::standard: {
                db(sqlpp::update(tables::users_relax_table).set(
                    tables::users_relax_table.pp_std = this->stats.pp,
                    tables::users_relax_table.total_score_std = this->stats.total_score,
                    tables::users_relax_table.ranked_score_std = this->stats.ranked_score,
                    tables::users_relax_table.play_count_std = this->stats.play_count,
                    tables::users_relax_table.rank_std = this->stats.rank,
                    tables::users_relax_table.max_combo_std = this->stats.max_combo,
                    tables::users_relax_table.avg_accuracy_std = this->stats.accuracy,
                    tables::users_relax_table.total_hits_std = this->stats.total_hits,
                    tables::users_relax_table.play_time_std = this->stats.play_time,
                    tables::users_relax_table.count_A_std = this->stats.count_A,
                    tables::users_relax_table.count_S_std = this->stats.count_S,
                    tables::users_relax_table.count_X_std = this->stats.count_X,
                    tables::users_relax_table.count_SH_std = this->stats.count_SH,
                    tables::users_relax_table.count_XH_std = this->stats.count_XH
                ).where(tables::users_relax_table.id == this->user_id));
                break;
            }
            case utils::play_mode::taiko: {
                db(sqlpp::update(tables::users_relax_table).set(
                    tables::users_relax_table.pp_taiko = this->stats.pp,
                    tables::users_relax_table.total_score_taiko = this->stats.total_score,
                    tables::users_relax_table.ranked_score_taiko = this->stats.ranked_score,
                    tables::users_relax_table.play_count_taiko = this->stats.play_count,
                    tables::users_relax_table.rank_taiko = this->stats.rank,
                    tables::users_relax_table.max_combo_taiko = this->stats.max_combo,
                    tables::users_relax_table.avg_accuracy_taiko = this->stats.accuracy,
                    tables::users_relax_table.total_hits_taiko = this->stats.total_hits,
                    tables::users_relax_table.play_time_taiko = this->stats.play_time,
                    tables::users_relax_table.count_A_taiko = this->stats.count_A,
                    tables::users_relax_table.count_S_taiko = this->stats.count_S,
                    tables::users_relax_table.count_X_taiko = this->stats.count_X,
                    tables::users_relax_table.count_SH_taiko = this->stats.count_SH,
                    tables::users_relax_table.count_XH_taiko = this->stats.count_XH
                ).where(tables::users_relax_table.id == this->user_id));
                break;
            }
            case utils::play_mode::fruits: {
                db(sqlpp::update(tables::users_relax_table).set(
                    tables::users_relax_table.pp_ctb = this->stats.pp,
                    tables::users_relax_table.total_score_ctb = this->stats.total_score,
                    tables::users_relax_table.ranked_score_ctb = this->stats.ranked_score,
                    tables::users_relax_table.play_count_ctb = this->stats.play_count,
                    tables::users_relax_table.rank_ctb = this->stats.rank,
                    tables::users_relax_table.max_combo_ctb = this->stats.max_combo,
                    tables::users_relax_table.avg_accuracy_ctb = this->stats.accuracy,
                    tables::users_relax_table.total_hits_ctb = this->stats.total_hits,
                    tables::users_relax_table.play_time_ctb = this->stats.play_time,
                    tables::users_relax_table.count_A_ctb = this->stats.count_A,
                    tables::users_relax_table.count_S_ctb = this->stats.count_S,
                    tables::users_relax_table.count_X_ctb = this->stats.count_X,
                    tables::users_relax_table.count_SH_ctb = this->stats.count_SH,
                    tables::users_relax_table.count_XH_ctb = this->stats.count_XH
                ).where(tables::users_relax_table.id == this->user_id));
                break;
            }
            case utils::play_mode::mania: {
                break;
            }
        }

        return;
    }

    switch (static_cast<utils::play_mode>(this->stats.play_mode)) {
        case utils::play_mode::standard: {
            db(sqlpp::update(tables::users_stats_table).set(
                tables::users_stats_table.pp_std = this->stats.pp,
                tables::users_stats_table.total_score_std = this->stats.total_score,
                tables::users_stats_table.ranked_score_std = this->stats.ranked_score,
                tables::users_stats_table.play_count_std = this->stats.play_count,
                tables::users_stats_table.rank_std = this->stats.rank,
                tables::users_stats_table.max_combo_std = this->stats.max_combo,
                tables::users_stats_table.avg_accuracy_std = this->stats.accuracy,
                tables::users_stats_table.total_hits_std = this->stats.total_hits,
                tables::users_stats_table.play_time_std = this->stats.play_time,
                tables::users_stats_table.count_A_std = this->stats.count_A,
                tables::users_stats_table.count_S_std = this->stats.count_S,
                tables::users_stats_table.count_X_std = this->stats.count_X,
                tables::users_stats_table.count_SH_std = this->stats.count_SH,
                tables::users_stats_table.count_XH_std = this->stats.count_XH
            ).where(tables::users_stats_table.id == this->user_id));
            break;
        }
        case utils::play_mode::taiko: {
            db(sqlpp::update(tables::users_stats_table).set(
                tables::users_stats_table.pp_taiko = this->stats.pp,
                tables::users_stats_table.total_score_taiko = this->stats.total_score,
                tables::users_stats_table.ranked_score_taiko = this->stats.ranked_score,
                tables::users_stats_table.play_count_taiko = this->stats.play_count,
                tables::users_stats_table.rank_taiko = this->stats.rank,
                tables::users_stats_table.max_combo_taiko = this->stats.max_combo,
                tables::users_stats_table.avg_accuracy_taiko = this->stats.accuracy,
                tables::users_stats_table.total_hits_taiko = this->stats.total_hits,
                tables::users_stats_table.play_time_taiko = this->stats.play_time,
                tables::users_stats_table.count_A_taiko = this->stats.count_A,
                tables::users_stats_table.count_S_taiko = this->stats.count_S,
                tables::users_stats_table.count_X_taiko = this->stats.count_X,
                tables::users_stats_table.count_SH_taiko = this->stats.count_SH,
                tables::users_stats_table.count_XH_taiko = this->stats.count_XH
            ).where(tables::users_stats_table.id == this->user_id));
            break;
        }
        case utils::play_mode::fruits: {
            db(sqlpp::update(tables::users_stats_table).set(
                tables::users_stats_table.pp_ctb = this->stats.pp,
                tables::users_stats_table.total_score_ctb = this->stats.total_score,
                tables::users_stats_table.ranked_score_ctb = this->stats.ranked_score,
                tables::users_stats_table.play_count_ctb = this->stats.play_count,
                tables::users_stats_table.rank_ctb = this->stats.rank,
                tables::users_stats_table.max_combo_ctb = this->stats.max_combo,
                tables::users_stats_table.avg_accuracy_ctb = this->stats.accuracy,
                tables::users_stats_table.total_hits_ctb = this->stats.total_hits,
                tables::users_stats_table.play_time_ctb = this->stats.play_time,
                tables::users_stats_table.count_A_ctb = this->stats.count_A,
                tables::users_stats_table.count_S_ctb = this->stats.count_S,
                tables::users_stats_table.count_X_ctb = this->stats.count_X,
                tables::users_stats_table.count_SH_ctb = this->stats.count_SH,
                tables::users_stats_table.count_XH_ctb = this->stats.count_XH
            ).where(tables::users_stats_table.id == this->user_id));
            break;
        }
        case utils::play_mode::mania: {
            db(sqlpp::update(tables::users_stats_table).set(
                tables::users_stats_table.pp_mania = this->stats.pp,
                tables::users_stats_table.total_score_mania = this->stats.total_score,
                tables::users_stats_table.ranked_score_mania = this->stats.ranked_score,
                tables::users_stats_table.play_count_mania = this->stats.play_count,
                tables::users_stats_table.rank_mania = this->stats.rank,
                tables::users_stats_table.max_combo_mania = this->stats.max_combo,
                tables::users_stats_table.avg_accuracy_mania = this->stats.accuracy,
                tables::users_stats_table.total_hits_mania = this->stats.total_hits,
                tables::users_stats_table.play_time_mania = this->stats.play_time,
                tables::users_stats_table.count_A_mania = this->stats.count_A,
                tables::users_stats_table.count_S_mania = this->stats.count_S,
                tables::users_stats_table.count_X_mania = this->stats.count_X,
                tables::users_stats_table.count_SH_mania = this->stats.count_SH,
                tables::users_stats_table.count_XH_mania = this->stats.count_XH
            ).where(tables::users_stats_table.id == this->user_id));
            break;
        }
    }
}

void shiro::users::user::update_country(const std::string& country) {
    this->country = country;

    auto db = shiro::database::instance->pop();
    db(sqlpp::update(tables::users_table).set(tables::users_table.country = country).where(tables::users_table.id == this->user_id));
}

void shiro::users::user::update_counts(const std::string& rank) {
    const static std::unordered_map<std::string, std::function<void()>> ranks = {
        { "A",  [&]() { this->stats.count_A++;  } },
        { "S",  [&]() { this->stats.count_S++;  } },
        { "X",  [&]() { this->stats.count_X++;  } },
        { "SH", [&]() { this->stats.count_SH++; } },
        { "XH", [&]() { this->stats.count_XH++; } }
    };

    auto it = ranks.find(rank);
    if (it != ranks.end()) {
        it->second();
    }
}

std::string shiro::users::user::get_url() {
    return config::ipc::frontend_url + "u/" + std::to_string(this->user_id);
}

std::string shiro::users::user::get_avatar_url() {
    return config::ipc::avatar_url + std::to_string(this->user_id);
}

void shiro::users::user::refresh_stats() {
    io::osu_writer writer;

    writer.user_stats(this->stats);
    writer.user_presence(this->presence);

    this->queue.enqueue(writer);
}

bool shiro::users::user::check_password(const std::string &password) {
    if (this->password.empty() || password.empty()) {
        return false;
    }

    return utils::crypto::pbkdf2_hmac_sha512::hash(password, this->salt) == this->password;
}

void shiro::users::user::verify_address(const std::string& address) {
    auto db = shiro::database::instance->pop();

    auto ip = db(sqlpp::select(tables::users_table.ip).from(tables::users_table).where(tables::users_table.id == this->user_id)).front().ip;
    std::string ip_string = ip.value();

    if (address == ip_string) {
        return;
    }

    LOG_F(WARNING, "The IP address of user {} has been changed from {} to {}.", this->presence.username, ip_string, address);
    // TODO: Add table where every IP change will be recorded, even better will be with HWID
    db(sqlpp::update(tables::users_table).set(tables::users_table.ip = address).where(tables::users_table.id == this->user_id));
}

shiro::users::user_preferences::user_preferences(int32_t id) {
    auto db = shiro::database::instance->pop();

    auto result = db(sqlpp::select(sqlpp::all_of(tables::users_preferences_table)).from(tables::users_preferences_table).where(tables::users_preferences_table.id == id));
    if (result.empty()) {
        return;
    }

    auto& data = result.front();
    this->display_classic = data.scoreboard_display_classic;
    this->display_relax = data.scoreboard_display_relax;
    this->auto_classic = data.auto_last_classic;
    this->auto_relax = data.auto_last_relax;
    this->score_ow_std = data.score_overwrite_std;
    this->score_ow_taiko = data.score_overwrite_taiko;
    this->score_ow_ctb = data.score_overwrite_ctb;
    this->score_ow_mania = data.score_overwrite_mania;
}

bool shiro::users::user_preferences::is_overwrite(shiro::utils::play_mode mode) {
    switch (mode) {
        case shiro::utils::play_mode::standard: {
            return this->score_ow_std;
        }
        case shiro::utils::play_mode::taiko: {
            return this->score_ow_taiko;
        }
        case shiro::utils::play_mode::fruits: {
            return this->score_ow_ctb;
        }
        case shiro::utils::play_mode::mania: {
            return this->score_ow_mania;
        }
    }

    return false;
}
