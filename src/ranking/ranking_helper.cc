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

#include "../database/tables/user_table.hh"
#include "../pp/pp_recalculator.hh"
#include "../users/user_activity.hh"
#include "../users/user_manager.hh"
#include "../users/user_punishments.hh"
#include "../utils/play_mode.hh"
#include "ranking_helper.hh"

int32_t shiro::ranking::helper::get_leaderboard_position(uint8_t mode, std::string username, bool is_relax) {
    if (username.empty()) {
        return 0;
    }

    auto db = shiro::database::instance->pop();
    std::vector<std::pair<std::string, float>> users;

    if (is_relax) {
        auto result = db(select(
            tables::users_relax_table.id,
            tables::users_table.username,
            tables::users_relax_table.pp_std,
            tables::users_relax_table.pp_taiko,
            tables::users_relax_table.pp_ctb
        ).from(tables::users_relax_table.join(tables::users_table).on(tables::users_relax_table.id == tables::users_table.id)).unconditionally());

        if (result.empty()) {
            return 0;
        }

        for (const auto& row : result) {
            if (row.id.value() == 1) {
                continue;
            }

            if (!users::punishments::has_scores(row.id)) {
                continue;
            }

            switch (static_cast<utils::play_mode>(mode)) {
                case utils::play_mode::standard: {
                    users.emplace_back(std::make_pair<std::string, float>(row.username, row.pp_std));
                    break;
                }
                case utils::play_mode::taiko: {
                    users.emplace_back(std::make_pair<std::string, float>(row.username, row.pp_taiko));
                    break;
                }
                case utils::play_mode::fruits: {
                    users.emplace_back(std::make_pair<std::string, float>(row.username, row.pp_ctb));
                    break;
                }
                case utils::play_mode::mania: {
                    return 0;
                }
            }
        }
    }
    else {
        auto result = db(select(
            tables::users_stats_table.id,
            tables::users_table.username,
            tables::users_stats_table.pp_std,
            tables::users_stats_table.pp_taiko,
            tables::users_stats_table.pp_ctb,
            tables::users_stats_table.pp_mania
        ).from(tables::users_stats_table.join(tables::users_table).on(tables::users_stats_table.id == tables::users_table.id)).unconditionally());

        if (result.empty()) {
            return 0;
        }

        for (const auto& row : result) {
            if (row.id.value() == 1) {
                continue;
            }

            if (!users::punishments::has_scores(row.id)) {
                continue;
            }

            switch (static_cast<utils::play_mode>(mode)) {
                case utils::play_mode::standard: {
                    users.emplace_back(std::make_pair<std::string, float>(row.username, row.pp_std));
                    break;
                }
                case utils::play_mode::taiko: {
                    users.emplace_back(std::make_pair<std::string, float>(row.username, row.pp_taiko));
                    break;
                }
                case utils::play_mode::fruits: {
                    users.emplace_back(std::make_pair<std::string, float>(row.username, row.pp_ctb));
                    break;
                }
                case utils::play_mode::mania: {
                    users.emplace_back(std::make_pair<std::string, float>(row.username, row.pp_mania));
                    break;
                }
            }
        }
    }

    std::sort(users.begin(), users.end(), [](const std::pair<std::string, float> &s_left, const std::pair<std::string, float> &s_right) {
        return s_left.second > s_right.second;
    });

    for (size_t i = 0; i < users.size(); i++) {
        const auto &[name, _] = users.at(i);

        if (username == name) {
            return i + 1;
        }
    }

    return 0;
}

std::string shiro::ranking::helper::get_leaderboard_user(uint8_t mode, size_t pos, bool is_relax) {
    if (pos == 0) {
        return "";
    }

    auto db = shiro::database::instance->pop();
    std::vector<std::pair<std::string, float>> users;

    if (is_relax) {
        auto result = db(select(
            tables::users_stats_table.id,
            tables::users_table.username,
            tables::users_stats_table.pp_std,
            tables::users_stats_table.pp_taiko,
            tables::users_stats_table.pp_ctb
        ).from(tables::users_stats_table.join(tables::users_table).on(tables::users_stats_table.id == tables::users_table.id)).unconditionally());

        if (result.empty()) {
            return "";
        }


        for (const auto& row : result) {
            if (row.id.value() == 1) {
                continue;
            }

            if (!users::punishments::has_scores(row.id)) {
                continue;
            }

            switch (static_cast<utils::play_mode>(mode)) {
                case utils::play_mode::standard: {
                    users.emplace_back(std::make_pair<std::string, float>(row.username, row.pp_std));
                    break;
                }
                case utils::play_mode::taiko: {
                    users.emplace_back(std::make_pair<std::string, float>(row.username, row.pp_taiko));
                    break;
                }
                case utils::play_mode::fruits: {
                    users.emplace_back(std::make_pair<std::string, float>(row.username, row.pp_ctb));
                    break;
                }
                case utils::play_mode::mania: {
                    return "";
                }
            }
        }
    }
    else {
        auto result = db(select(
            tables::users_stats_table.id,
            tables::users_table.username,
            tables::users_stats_table.pp_std,
            tables::users_stats_table.pp_taiko,
            tables::users_stats_table.pp_ctb,
            tables::users_stats_table.pp_mania
        ).from(tables::users_stats_table.join(tables::users_table).on(tables::users_stats_table.id == tables::users_table.id)).unconditionally());

        if (result.empty()) {
            return "";
        }


        for (const auto& row : result) {
            if (row.id.value() == 1) {
                continue;
            }

            if (!users::punishments::has_scores(row.id)) {
                continue;
            }

            switch (static_cast<utils::play_mode>(mode)) {
                case utils::play_mode::standard: {
                    users.emplace_back(std::make_pair<std::string, float>(row.username, row.pp_std));
                    break;
                }
                case utils::play_mode::taiko: {
                    users.emplace_back(std::make_pair<std::string, float>(row.username, row.pp_taiko));
                    break;
                }
                case utils::play_mode::fruits: {
                    users.emplace_back(std::make_pair<std::string, float>(row.username, row.pp_ctb));
                    break;
                }
                case utils::play_mode::mania: {
                    users.emplace_back(std::make_pair<std::string, float>(row.username, row.pp_mania));
                    break;
                }
            }
        }
    }

    std::sort(users.begin(), users.end(), [](const std::pair<std::string, float> &s_left, const std::pair<std::string, float> &s_right) {
        return s_left.second > s_right.second;
    });

    if (users.size() < pos) {
        return "";
    }

    return users.at(pos - 1).first;
}

int16_t shiro::ranking::helper::get_pp_for_user(uint8_t mode, std::string username, bool is_relax) {
    if (username.empty()) {
        return 0;
    }

    auto db = shiro::database::instance->pop();

    if (is_relax) {
        auto result = db(select(
            tables::users_stats_table.pp_std,
            tables::users_stats_table.pp_taiko,
            tables::users_stats_table.pp_ctb,
            tables::users_table.username
        ).from(tables::users_table.join(tables::users_stats_table).on(tables::users_table.id == tables::users_stats_table.id))
            .where(tables::users_table.username == username).limit(1u));

        if (result.empty()) {
            return 0;
        }

        const auto& row = result.front();

        switch (static_cast<utils::play_mode>(mode)) {
            case utils::play_mode::standard: {
                return row.pp_std;
            }
            case utils::play_mode::taiko: {
                return row.pp_taiko;
            }
            case utils::play_mode::fruits: {
                return row.pp_ctb;
            }
            case utils::play_mode::mania: {
                return 0;
            }
        }
    }
    else {
        auto result = db(select(
            tables::users_stats_table.pp_std,
            tables::users_stats_table.pp_taiko,
            tables::users_stats_table.pp_ctb,
            tables::users_stats_table.pp_mania,
            tables::users_table.username
        ).from(tables::users_table.join(tables::users_stats_table).on(tables::users_table.id == tables::users_stats_table.id))
            .where(tables::users_table.username == username).limit(1u));

        if (result.empty()) {
            return 0;
        }

        const auto& row = result.front();

        switch (static_cast<utils::play_mode>(mode)) {
            case utils::play_mode::standard: {
                return row.pp_std;
            }
            case utils::play_mode::taiko: {
                return row.pp_taiko;
            }
            case utils::play_mode::fruits: {
                return row.pp_ctb;
            }
            case utils::play_mode::mania: {
                return row.pp_mania;
            }
        }
    }

    return 0;
}

void shiro::ranking::helper::recalculate_ranks(const shiro::utils::play_mode &mode, bool is_relax) {
    // Global pp recalculation is currently in progress.
    if (pp::recalculator::in_progress()) {
        return;
    }

    auto db = shiro::database::instance->pop();
    std::vector<std::pair<int32_t, float>> users;

    if (is_relax) {
        auto result = db(select(
            tables::users_stats_table.id,
            tables::users_stats_table.pp_std,
            tables::users_stats_table.pp_taiko,
            tables::users_stats_table.pp_ctb,
            tables::users_stats_table.play_count_std,
            tables::users_stats_table.play_count_taiko,
            tables::users_stats_table.play_count_ctb
        ).from(tables::users_stats_table).unconditionally());

        if (result.empty()) {
            return;
        }

        for (const auto& row : result) {
            if (row.id.value() == 1) {
                continue;
            }

            // This needs to be configurable for future updates
            // Users should be purged once there is a update to the performance calculator
            // They shouldn't be purged *every* time a new score is submitted
            //if (users::activity::is_inactive(row.id, mode))
            //    continue;

            if (!users::punishments::has_scores(row.id)) {
                continue;
            }

            switch (mode) {
                case utils::play_mode::standard: {
                    if (row.play_count_std.value() <= 0) {
                        continue;
                    }

                    users.emplace_back(std::make_pair<int32_t, float>(row.id, row.pp_std));
                    break;
                }
                case utils::play_mode::taiko: {
                    if (row.play_count_taiko.value() <= 0) {
                        continue;
                    }

                    users.emplace_back(std::make_pair<int32_t, float>(row.id, row.pp_taiko));
                    break;
                }
                case utils::play_mode::fruits: {
                    if (row.play_count_ctb.value() <= 0) {
                        continue;
                    }

                    users.emplace_back(std::make_pair<int32_t, float>(row.id, row.pp_ctb));
                    break;
                }
                case utils::play_mode::mania: {
                    // Useless calculations
                    return;
                }
            }
        }
    }
    else {
        auto result = db(select(
            tables::users_stats_table.id,
            tables::users_stats_table.pp_std,
            tables::users_stats_table.pp_taiko,
            tables::users_stats_table.pp_ctb,
            tables::users_stats_table.pp_mania,
            tables::users_stats_table.play_count_std,
            tables::users_stats_table.play_count_taiko,
            tables::users_stats_table.play_count_ctb,
            tables::users_stats_table.play_count_mania
        ).from(tables::users_stats_table).unconditionally());

        if (result.empty()) {
            return;
        }

        for (const auto& row : result) {
            if (row.id.value() == 1) {
                continue;
            }

            // This needs to be configurable for future updates
            // Users should be purged once there is a update to the performance calculator
            // They shouldn't be purged *every* time a new score is submitted
            //if (users::activity::is_inactive(row.id, mode))
            //    continue;

            if (!users::punishments::has_scores(row.id)) {
                continue;
            }

            switch (mode) {
                case utils::play_mode::standard: {
                    if (row.play_count_std.value() <= 0) {
                        continue;
                    }

                    users.emplace_back(std::make_pair<int32_t, float>(row.id, row.pp_std));
                    break;
                }
                case utils::play_mode::taiko: {
                    if (row.play_count_taiko.value() <= 0) {
                        continue;
                    }

                    users.emplace_back(std::make_pair<int32_t, float>(row.id, row.pp_taiko));
                    break;
                }
                case utils::play_mode::fruits: {
                    if (row.play_count_ctb.value() <= 0) {
                        continue;
                    }

                    users.emplace_back(std::make_pair<int32_t, float>(row.id, row.pp_ctb));
                    break;
                }
                case utils::play_mode::mania: {
                    if (row.play_count_mania.value() <= 0) {
                        continue;
                    }

                    users.emplace_back(std::make_pair<int32_t, float>(row.id, row.pp_mania));
                    break;
                }
            }
        }
    }

    std::sort(users.begin(), users.end(), [](const std::pair<int32_t, float> &s_left, const std::pair<int32_t, float> &s_right) {
        return s_left.second > s_right.second;
    });

    // At this point, the users array is sorted by rank, this means the 0th element is rank #1

    if (is_relax) {
        for (size_t i = 0; i < users.size(); i++) {
            auto [user_id, pp] = users.at(i);
            int32_t rank = static_cast<int32_t>(i) + 1;

            std::shared_ptr<users::user> user = users::manager::get_user_by_id(user_id);
            if (user != nullptr && user->stats.play_mode == static_cast<uint8_t>(mode)) {
                user->stats.rank = rank;
                user->presence.rank = rank;
            }

            switch (mode) {
                case utils::play_mode::standard: {
                    db(update(tables::users_relax_table).set(tables::users_relax_table.rank_std = rank).where(tables::users_relax_table.id == user_id));
                    break;
                }
                case utils::play_mode::taiko: {
                    db(update(tables::users_relax_table).set(tables::users_relax_table.rank_taiko = rank).where(tables::users_relax_table.id == user_id));
                    break;
                }
                case utils::play_mode::fruits: {
                    db(update(tables::users_relax_table).set(tables::users_relax_table.rank_ctb = rank).where(tables::users_relax_table.id == user_id));
                    break;
                }
                case utils::play_mode::mania: {
                    // Useless calculations
                    return;
                }
            }
        }
    }
    else {
        for (size_t i = 0; i < users.size(); i++) {
            auto [user_id, pp] = users.at(i);
            int32_t rank = static_cast<int32_t>(i) + 1;

            std::shared_ptr<users::user> user = users::manager::get_user_by_id(user_id);
            if (user != nullptr && user->stats.play_mode == static_cast<uint8_t>(mode)) {
                user->stats.rank = rank;
                user->presence.rank = rank;
            }

            switch (mode) {
                case utils::play_mode::standard: {
                    db(update(tables::users_stats_table).set(tables::users_stats_table.rank_std = rank).where(tables::users_stats_table.id == user_id));
                    break;
                }
                case utils::play_mode::taiko: {
                    db(update(tables::users_stats_table).set(tables::users_stats_table.rank_taiko = rank).where(tables::users_stats_table.id == user_id));
                    break;
                }
                case utils::play_mode::fruits: {
                    db(update(tables::users_stats_table).set(tables::users_stats_table.rank_ctb = rank).where(tables::users_stats_table.id == user_id));
                    break;
                }
                case utils::play_mode::mania: {
                    db(update(tables::users_stats_table).set(tables::users_stats_table.rank_mania = rank).where(tables::users_stats_table.id == user_id));
                    break;
                }
            }
        }
    }

    io::osu_writer writer;

    // First we add all user stats/presence updates to the global writer
    users::manager::iterate([&writer](std::shared_ptr<users::user> online_user) {
        if (online_user->hidden) {
            return;
        }

        writer.user_stats(online_user->stats);
        writer.user_presence(online_user->presence);
    }, true);

    // After we have all user updates in the writer, we can send them out globally
    users::manager::iterate([&writer](std::shared_ptr<users::user> online_user) {
        online_user->queue.enqueue(writer);
    }, true);
}
