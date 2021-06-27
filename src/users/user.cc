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
#include "../database/tables/relationship_table.hh"
#include "../database/tables/user_table.hh"
#include "../permissions/role_manager.hh"
#include "../thirdparty/loguru.hh"
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
    sqlpp::mysql::connection db(db_connection->get_config());
    const tables::users user_table {};
    const tables::relationships relationships_table {};

    auto user_result = db(select(all_of(user_table)).from(user_table).where(user_table.id == this->user_id or user_table.username == this->presence.username).limit(1u));

    if (user_result.empty())
        return false;

    const auto &row = user_result.front();

    this->user_id = row.id;
    this->country = row.country;
    this->presence.user_id = this->user_id;
    this->stats.user_id = this->user_id;
    this->presence.username = row.username;
    this->password = row.password_md5;
    this->salt = row.salt;
    this->roles = row.roles;
    this->presence.permissions = roles::manager::get_chat_color(this->roles);
    this->isRelax = row.is_relax;

    auto relationship_result = db(select(all_of(relationships_table)).from(relationships_table).where(relationships_table.origin == this->user_id and relationships_table.blocked == false));

    for (const auto &row : relationship_result) {
        this->friends.emplace_back(row.target);
    }

    if (users::punishments::is_restricted(this->user_id))
        this->hidden = true;

    if (isRelax)
    {
        const tables::users_stats_relax user_stats_table {};

        auto stats_result = db(sqlpp::select(
            user_stats_table.pp_std,
            user_stats_table.max_combo_std,
            user_stats_table.avg_accuracy_std,
            user_stats_table.total_score_std,
            user_stats_table.ranked_score_std,
            user_stats_table.play_count_std,
            user_stats_table.rank_std,
            user_stats_table.play_time_std
        ).from(user_stats_table).where(user_stats_table.id == this->user_id));

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

    const tables::users_stats user_stats_table {};

    auto stats_result = db(sqlpp::select(
        user_stats_table.pp_std,
        user_stats_table.max_combo_std,
        user_stats_table.avg_accuracy_std,
        user_stats_table.total_score_std,
        user_stats_table.ranked_score_std,
        user_stats_table.play_count_std,
        user_stats_table.rank_std,
        user_stats_table.play_time_std
    ).from(user_stats_table).where(user_stats_table.id == this->user_id));

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

void shiro::users::user::update(bool isRelax) {
    sqlpp::mysql::connection db(db_connection->get_config());

    const tables::users user_table {};
    db(sqlpp::update(user_table).set(user_table.is_relax = isRelax).where(user_table.id == this->user_id));

    if (isRelax)
    {
        const tables::users_stats_relax user_stats_table {};
        auto result = db(select(all_of(user_stats_table)).from(user_stats_table).where(user_stats_table.id == this->user_id).limit(1u));

        if (result.empty())
            return;

        const auto& row = result.front();

        if (this->status.play_mode == (uint8_t)utils::play_mode::standard) {
            this->stats.pp = row.pp_std;
            this->stats.total_score = row.total_score_std;
            this->stats.ranked_score = row.ranked_score_std;
            this->stats.play_count = row.play_count_std;
            this->stats.rank = row.rank_std;
            this->presence.rank = row.rank_std;
            this->stats.max_combo = row.max_combo_std;
            this->stats.accuracy = row.avg_accuracy_std;
            this->stats.play_time = row.play_time_std;
        }
        else if (this->status.play_mode == (uint8_t)utils::play_mode::taiko) {
            this->stats.pp = row.pp_taiko;
            this->stats.total_score = row.total_score_taiko;
            this->stats.ranked_score = row.ranked_score_taiko;
            this->stats.play_count = row.play_count_taiko;
            this->stats.rank = row.rank_taiko;
            this->presence.rank = row.rank_taiko;
            this->stats.max_combo = row.max_combo_taiko;
            this->stats.accuracy = row.avg_accuracy_taiko;
            this->stats.play_time = row.play_time_taiko;
        }
        else if (this->status.play_mode == (uint8_t)utils::play_mode::fruits) {
            this->stats.pp = row.pp_ctb;
            this->stats.total_score = row.total_score_ctb;
            this->stats.ranked_score = row.ranked_score_ctb;
            this->stats.play_count = row.play_count_ctb;
            this->stats.rank = row.rank_ctb;
            this->presence.rank = row.rank_ctb;
            this->stats.max_combo = row.max_combo_ctb;
            this->stats.accuracy = row.avg_accuracy_ctb;
            this->stats.play_time = row.play_time_ctb;
        }

        this->isRelax = true;
        return;
    }

    const tables::users_stats user_stats_table {};
    auto result = db(select(all_of(user_stats_table)).from(user_stats_table).where(user_stats_table.id == this->user_id).limit(1u));

    if (result.empty())
        return;

    const auto& row = result.front();

    if (this->status.play_mode == (uint8_t)utils::play_mode::standard) {
        this->stats.pp = row.pp_std;
        this->stats.total_score = row.total_score_std;
        this->stats.ranked_score = row.ranked_score_std;
        this->stats.play_count = row.play_count_std;
        this->stats.rank = row.rank_std;
        this->presence.rank = row.rank_std;
        this->stats.max_combo = row.max_combo_std;
        this->stats.accuracy = row.avg_accuracy_std;
        this->stats.play_time = row.play_time_std;
    }
    else if (this->status.play_mode == (uint8_t)utils::play_mode::taiko) {
        this->stats.pp = row.pp_taiko;
        this->stats.total_score = row.total_score_taiko;
        this->stats.ranked_score = row.ranked_score_taiko;
        this->stats.play_count = row.play_count_taiko;
        this->stats.rank = row.rank_taiko;
        this->presence.rank = row.rank_taiko;
        this->stats.max_combo = row.max_combo_taiko;
        this->stats.accuracy = row.avg_accuracy_taiko;
        this->stats.play_time = row.play_time_taiko;
    }
    else if (this->status.play_mode == (uint8_t)utils::play_mode::fruits) {
        this->stats.pp = row.pp_ctb;
        this->stats.total_score = row.total_score_ctb;
        this->stats.ranked_score = row.ranked_score_ctb;
        this->stats.play_count = row.play_count_ctb;
        this->stats.rank = row.rank_ctb;
        this->presence.rank = row.rank_ctb;
        this->stats.max_combo = row.max_combo_ctb;
        this->stats.accuracy = row.avg_accuracy_ctb;
        this->stats.play_time = row.play_time_ctb;
    }
    else if (this->status.play_mode == (uint8_t)utils::play_mode::mania) {
        this->stats.pp = row.pp_mania;
        this->stats.total_score = row.total_score_mania;
        this->stats.ranked_score = row.ranked_score_mania;
        this->stats.play_count = row.play_count_mania;
        this->stats.rank = row.rank_mania;
        this->presence.rank = row.rank_mania;
        this->stats.max_combo = row.max_combo_mania;
        this->stats.accuracy = row.avg_accuracy_mania;
        this->stats.play_time = row.play_time_mania;
    }

    this->isRelax = false;
}

void shiro::users::user::save_stats(bool toRelax) {
    sqlpp::mysql::connection db(db_connection->get_config());
    
    if (toRelax)
    {
        const tables::users_stats_relax user_stats_table {};
        switch (this->stats.play_mode) {
        case (uint8_t)utils::play_mode::standard:
            db(sqlpp::update(user_stats_table).set(
                user_stats_table.pp_std = this->stats.pp,
                user_stats_table.total_score_std = this->stats.total_score,
                user_stats_table.ranked_score_std = this->stats.ranked_score,
                user_stats_table.play_count_std = this->stats.play_count,
                user_stats_table.rank_std = this->stats.rank,
                user_stats_table.max_combo_std = this->stats.max_combo,
                user_stats_table.avg_accuracy_std = this->stats.accuracy,
                user_stats_table.play_time_std = this->stats.play_time
            ).where(user_stats_table.id == this->user_id));
            break;
        case (uint8_t)utils::play_mode::taiko:
            db(sqlpp::update(user_stats_table).set(
                user_stats_table.pp_taiko = this->stats.pp,
                user_stats_table.total_score_taiko = this->stats.total_score,
                user_stats_table.ranked_score_taiko = this->stats.ranked_score,
                user_stats_table.play_count_taiko = this->stats.play_count,
                user_stats_table.rank_taiko = this->stats.rank,
                user_stats_table.max_combo_taiko = this->stats.max_combo,
                user_stats_table.avg_accuracy_taiko = this->stats.accuracy,
                user_stats_table.play_time_taiko = this->stats.play_time
            ).where(user_stats_table.id == this->user_id));
            break;
        case (uint8_t)utils::play_mode::fruits:
            db(sqlpp::update(user_stats_table).set(
                user_stats_table.pp_ctb = this->stats.pp,
                user_stats_table.total_score_ctb = this->stats.total_score,
                user_stats_table.ranked_score_ctb = this->stats.ranked_score,
                user_stats_table.play_count_ctb = this->stats.play_count,
                user_stats_table.rank_ctb = this->stats.rank,
                user_stats_table.max_combo_ctb = this->stats.max_combo,
                user_stats_table.avg_accuracy_ctb = this->stats.accuracy,
                user_stats_table.play_time_ctb = this->stats.play_time
            ).where(user_stats_table.id == this->user_id));
            break;
        }

        return;
    }

    const tables::users_stats user_stats_table {};

    switch (this->stats.play_mode) {
    case (uint8_t)utils::play_mode::standard:
        db(sqlpp::update(user_stats_table).set(
            user_stats_table.pp_std = this->stats.pp,
            user_stats_table.total_score_std = this->stats.total_score,
            user_stats_table.ranked_score_std = this->stats.ranked_score,
            user_stats_table.play_count_std = this->stats.play_count,
            user_stats_table.rank_std = this->stats.rank,
            user_stats_table.max_combo_std = this->stats.max_combo,
            user_stats_table.avg_accuracy_std = this->stats.accuracy,
            user_stats_table.play_time_std = this->stats.play_time
        ).where(user_stats_table.id == this->user_id));
        break;
    case (uint8_t)utils::play_mode::taiko:
        db(sqlpp::update(user_stats_table).set(
            user_stats_table.pp_taiko = this->stats.pp,
            user_stats_table.total_score_taiko = this->stats.total_score,
            user_stats_table.ranked_score_taiko = this->stats.ranked_score,
            user_stats_table.play_count_taiko = this->stats.play_count,
            user_stats_table.rank_taiko = this->stats.rank,
            user_stats_table.max_combo_taiko = this->stats.max_combo,
            user_stats_table.avg_accuracy_taiko = this->stats.accuracy,
            user_stats_table.play_time_taiko = this->stats.play_time
        ).where(user_stats_table.id == this->user_id));
        break;
    case (uint8_t)utils::play_mode::fruits:
        db(sqlpp::update(user_stats_table).set(
            user_stats_table.pp_ctb = this->stats.pp,
            user_stats_table.total_score_ctb = this->stats.total_score,
            user_stats_table.ranked_score_ctb = this->stats.ranked_score,
            user_stats_table.play_count_ctb = this->stats.play_count,
            user_stats_table.rank_ctb = this->stats.rank,
            user_stats_table.max_combo_ctb = this->stats.max_combo,
            user_stats_table.avg_accuracy_ctb = this->stats.accuracy,
            user_stats_table.play_time_ctb = this->stats.play_time
        ).where(user_stats_table.id == this->user_id));
        break;
    case (uint8_t)utils::play_mode::mania:
        db(sqlpp::update(user_stats_table).set(
            user_stats_table.pp_mania = this->stats.pp,
            user_stats_table.total_score_mania = this->stats.total_score,
            user_stats_table.ranked_score_mania = this->stats.ranked_score,
            user_stats_table.play_count_mania = this->stats.play_count,
            user_stats_table.rank_mania = this->stats.rank,
            user_stats_table.max_combo_mania = this->stats.max_combo,
            user_stats_table.avg_accuracy_mania = this->stats.accuracy,
            user_stats_table.play_time_mania = this->stats.play_time
        ).where(user_stats_table.id == this->user_id));
        break;
    }
}

void shiro::users::user::update_country(std::string country)
{
    this->country = country;

    sqlpp::mysql::connection db(db_connection->get_config());
    const tables::users user_table{};

    db(sqlpp::update(user_table).set(user_table.country = country).where(user_table.id == this->user_id));
}

void shiro::users::user::update_play_time(uint8_t mode, bool isRelax)
{
    sqlpp::mysql::connection db(db_connection->get_config());

    if (isRelax)
    {
        const tables::users_stats_relax users_stats_table{};

        switch (mode)
        {
            case 1:
                db(sqlpp::update(users_stats_table).set(users_stats_table.play_time_taiko = this->stats.play_time).where(users_stats_table.id == this->user_id));
                return;
            case 2:
                db(sqlpp::update(users_stats_table).set(users_stats_table.play_time_ctb = this->stats.play_time).where(users_stats_table.id == this->user_id));
                return;
            case 0:
            default:
                db(sqlpp::update(users_stats_table).set(users_stats_table.play_time_std = this->stats.play_time).where(users_stats_table.id == this->user_id));
                return;
        }
    }

    const tables::users_stats users_stats_table{};

    switch (mode)
    {
        case 1:
            db(sqlpp::update(users_stats_table).set(users_stats_table.play_time_taiko = this->stats.play_time).where(users_stats_table.id == this->user_id));
            return;
        case 2:
            db(sqlpp::update(users_stats_table).set(users_stats_table.play_time_ctb = this->stats.play_time).where(users_stats_table.id == this->user_id));
            return;
        case 3:
            db(sqlpp::update(users_stats_table).set(users_stats_table.play_time_mania = this->stats.play_time).where(users_stats_table.id == this->user_id));
            return;
        case 0:
        default:
            db(sqlpp::update(users_stats_table).set(users_stats_table.play_time_std = this->stats.play_time).where(users_stats_table.id == this->user_id));
            return;
    }

}

std::string shiro::users::user::get_url() {
    static std::string url = config::ipc::frontend_url + "u/" + std::to_string(this->user_id);
    return url;
}

std::string shiro::users::user::get_avatar_url() 
{
    static std::string url = config::ipc::avatar_url + std::to_string(this->user_id);
    return url;
}

void shiro::users::user::refresh_stats() {
    io::osu_writer writer;

    writer.user_stats(this->stats);
    writer.user_presence(this->presence);

    this->queue.enqueue(writer);
}

bool shiro::users::user::check_password(const std::string &password) {
    if (this->password.empty() || password.empty())
        return false;

    return utils::crypto::pbkdf2_hmac_sha512::hash(password, this->salt) == this->password;
}
