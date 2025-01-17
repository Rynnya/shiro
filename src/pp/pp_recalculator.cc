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

#include "../database/tables/score_table.hh"
#include "../database/tables/user_table.hh"
#include "../beatmaps/beatmap_helper.hh"
#include "../ranking/ranking_helper.hh"
#include "../scores/score.hh"
#include "../scores/score_helper.hh"
#include "../thirdparty/naga.hh"
#include "../users/user.hh"
#include "../users/user_manager.hh"
#include "../utils/string_utils.hh"
#include "../utils/vector_utils.hh"
#include "../shiro.hh"
#include "pp_recalculator.hh"
#include "pp_score_metric.hh"

// We don't need locks, because all critical variables are atomic
// This also makes calculator works faster with more cores, as atomic always faster than locks (assuming that running this code machine will support one-op atomics)
std::atomic_bool shiro::pp::recalculator::running = false;
std::atomic_size_t shiro::pp::recalculator::running_threads = 0;

void shiro::pp::recalculator::begin(shiro::utils::play_mode mode, bool is_relax, uint32_t threads) {
    if (running) {
        LOG_F(ERROR, "PP recalculation was called while already recalculating.");
        return;
    }

    // According to https://en.cppreference.com/w/cpp/thread/thread/hardware_concurrency might return 0
    if (threads <= 0) {
        threads = 1;
    }

    if (threads >= 2) {
        threads /= 2; // Only use half of the available threads for pp recalculation
    }

    std::vector<int32_t> users;
    std::string game_mode = utils::play_mode_to_string(mode);

    auto db = shiro::database::instance->pop();

    if (is_relax) {
        auto result = db(select(
            tables::users_table.id,
            tables::users_table.permissions,
            tables::users_relax_table.play_count_std,
            tables::users_relax_table.play_count_taiko,
            tables::users_relax_table.play_count_ctb
        ).from(tables::users_table.join(tables::users_relax_table).on(tables::users_table.id == tables::users_relax_table.id)).unconditionally());

        for (const auto& row : result) {
            // Bots usually don't have scores, so let's skip them
            if (row.permissions.value() == 0xDEADCAFE) {
                continue;
            }

            int32_t play_count = 0;

            switch (mode) {
                case utils::play_mode::standard: {
                    play_count = row.play_count_std;
                    break;
                }
                case utils::play_mode::taiko: {
                    play_count = row.play_count_taiko;
                    break;
                }
                case utils::play_mode::fruits: {
                    play_count = row.play_count_ctb;
                    break;
                }
                case utils::play_mode::mania: {
                    return;
                }
            }

            // No need to recalculate users that don't have any scores
            if (play_count <= 0) {
                continue;
            }

            users.emplace_back(static_cast<int32_t>(row.id));
        }
    }
    else {
        auto result = db(select(
            tables::users_table.id,
            tables::users_table.permissions,
            tables::users_stats_table.play_count_std,
            tables::users_stats_table.play_count_taiko,
            tables::users_stats_table.play_count_ctb,
            tables::users_stats_table.play_count_mania
        ).from(tables::users_table.join(tables::users_stats_table).on(tables::users_table.id == tables::users_stats_table.id)).unconditionally());

        for (const auto& row : result) {
            // Bots usually don't have scores, so let's skip them
            if (row.permissions.value() == 0xDEADCAFE) {
                continue;
            }

            int32_t play_count = 0;

            switch (mode) {
                case utils::play_mode::standard: {
                    play_count = row.play_count_std;
                    break;
                }
                case utils::play_mode::taiko: {
                    play_count = row.play_count_taiko;
                    break;
                }
                case utils::play_mode::fruits: {
                    play_count = row.play_count_ctb;
                    break;
                }
                case utils::play_mode::mania: {
                    play_count = row.play_count_mania;
                    break;
                }
            }

            // No need to recalculate users that don't have any scores
            if (play_count <= 0) {
                continue;
            }

            users.emplace_back(static_cast<int32_t>(row.id));
        }
    }

    if (users.empty()) {
        return;
    }

    running = true;

    LOG_F(INFO, "Starting pp recalculation in {} for {} users with {} threads.", game_mode, users.size(), threads);

    bool evenly_distributable = users.size() % threads == 0;
    ptrdiff_t chunk_size = users.size() / threads;

    if (!evenly_distributable) {
        chunk_size += 1;
    }

    // If there are more (or equal the amount of) users than threads, every thread can calculate up to one user.
    if (threads >= users.size()) {
        chunk_size = 1;
        LOG_F(WARNING, "Thread amount is bigger or equal the amount of users.");
    }

    auto chunks = utils::chunk(users.begin(), users.end(), chunk_size);

    if (threads < users.size()) {
        LOG_F(INFO, "Distributing work across {} threads: every thread calculates up to {} users. ({} chunks)", threads, chunk_size, chunks.size());
    }

    for (size_t i = 0; i < chunks.size(); i++) {
        auto [begin, end] = chunks.at(i);

        std::vector<int32_t> chunked_users(begin, end);

        // We don't use thread_pool here because we need don't know amount of threads on compile-time
        std::thread thread(recalculate, mode, chunked_users, is_relax);
        running_threads++;

        LOG_F(INFO, "Thread {} started recalculating with chunk {}. ({} users)", naga::get_thread_name(), i, chunked_users.size());

        // Let the thread do it's work
        thread.detach();
    }

    io::osu_writer writer;
    writer.announce(fmt::format(
        "Global PP recalculation has begun for all scores in {} ({}). "
        "PP on scores may not match user overall pp amount. "
        "Global rank and user pp updates have been paused.",
        game_mode, (is_relax ? "Relax" : "Classic")
    ));

    users::manager::iterate([&writer](std::shared_ptr<users::user> user) {
        user->queue.enqueue(writer);
    }, true);

    LOG_F(INFO, "All recalculation threads have been started. Let's get this train rolling.");
}

void shiro::pp::recalculator::end(shiro::utils::play_mode mode, bool is_relax) {
    if (--running_threads > 0) {
        LOG_F(INFO, "Still {} threads running.", running_threads.load());
        return;
    }

    running = false;

    // Recalculate overall pp for all users now
    // TODO: This code is repeated in user_stats.cc, needs to be refactored asap
    auto db = shiro::database::instance->pop();

    if (is_relax) {
        auto result = db(select(
            tables::users_relax_table.id,
            tables::users_relax_table.pp_std,
            tables::users_relax_table.pp_taiko,
            tables::users_relax_table.pp_ctb
        ).from(tables::users_relax_table).unconditionally());

        for (const auto& row : result) {
            std::vector<scores::score> scores = scores::helper::fetch_top100_user(mode, row.id, is_relax);
            float raw_pp = 0; // Here it is a float to keep decimal points, round it when setting final pp value

            for (size_t i = 0; i < scores.size(); i++) {
                scores::score score = scores.at(i);

                raw_pp += (score.pp * std::pow(0.95, i));
            }

            std::shared_ptr<users::user> user = users::manager::get_user_by_id(row.id);
            int16_t pp = std::clamp(static_cast<int16_t>(raw_pp), static_cast<int16_t>(0), std::numeric_limits<int16_t>::max());
            if (user != nullptr && user->stats.play_mode == static_cast<uint8_t>(mode)) {
                user->stats.pp = pp;
            }

            switch (mode) {
                case utils::play_mode::standard: {
                    db(update(tables::users_relax_table).set(tables::users_relax_table.pp_std = pp).where(tables::users_relax_table.id == row.id));
                    break;
                }
                case utils::play_mode::taiko: {
                    db(update(tables::users_relax_table).set(tables::users_relax_table.pp_taiko = pp).where(tables::users_relax_table.id == row.id));
                    break;
                }
                case utils::play_mode::fruits: {
                    db(update(tables::users_relax_table).set(tables::users_relax_table.pp_ctb = pp).where(tables::users_relax_table.id == row.id));
                    break;
                }
                case utils::play_mode::mania: {
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
            tables::users_stats_table.pp_mania
        ).from(tables::users_stats_table).unconditionally());

        for (const auto& row : result) {
            std::vector<scores::score> scores = scores::helper::fetch_top100_user(mode, row.id, is_relax);
            float raw_pp = 0; // Here it is a float to keep decimal points, round it when setting final pp value

            for (size_t i = 0; i < scores.size(); i++) {
                scores::score score = scores.at(i);

                raw_pp += (score.pp * std::pow(0.95, i));
            }

            std::shared_ptr<users::user> user = users::manager::get_user_by_id(row.id);
            int16_t pp = std::clamp(static_cast<int16_t>(raw_pp), static_cast<int16_t>(0), std::numeric_limits<int16_t>::max());
            if (user != nullptr && user->stats.play_mode == static_cast<uint8_t>(mode)) {
                user->stats.pp = pp;
            }

            switch (mode) {
                case utils::play_mode::standard: {
                    db(update(tables::users_stats_table).set(tables::users_stats_table.pp_std = pp).where(tables::users_stats_table.id == row.id));
                    break;
                }
                case utils::play_mode::taiko: {
                    db(update(tables::users_stats_table).set(tables::users_stats_table.pp_taiko = pp).where(tables::users_stats_table.id == row.id));
                    break;
                }
                case utils::play_mode::fruits: {
                    db(update(tables::users_stats_table).set(tables::users_stats_table.pp_ctb = pp).where(tables::users_stats_table.id == row.id));
                    break;
                }
                case utils::play_mode::mania: {
                    db(update(tables::users_stats_table).set(tables::users_stats_table.pp_mania = pp).where(tables::users_stats_table.id == row.id));
                    break;
                }
            }
        }
    }

    // Recalculate global ranks now that user pp is updated
    ranking::helper::recalculate_ranks(mode, is_relax);

    io::osu_writer writer;
    writer.announce("PP recalculation has ended. Your pp amount and global rank have been updated.");

    users::manager::iterate([&writer](std::shared_ptr<users::user> user) {
        user->refresh_stats();
        user->queue.enqueue(writer);
    }, true);
}

bool shiro::pp::recalculator::in_progress() {
    return running;
}

void shiro::pp::recalculator::recalculate(shiro::utils::play_mode mode, std::vector<int32_t> users, bool is_relax) {
    auto db = shiro::database::instance->pop();

    for (int32_t user_id : users) {
        std::vector<scores::score> scores = scores::helper::fetch_all_user_scores(user_id, is_relax);

        if (scores.empty()) {
            continue;
        }

        for (const scores::score &score : scores) {
            beatmaps::beatmap map;
            map.beatmap_md5 = score.beatmap_md5;

            // On recalculation we would hit osu! API too often so we exclusively allow database fetching
            if (!map.fetch_db()) {
                // TODO: Create cache system that will download these maps after recalculate
                LOG_F(WARNING, "Score #{} was set on a beatmap that is not in the database. Skipping...", score.id);
                continue;
            }

            if (!beatmaps::helper::awards_pp(beatmaps::helper::fix_beatmap_status(map.ranked_status))) {
                continue;
            }

            float pp = pp::calculate(map, score);
            float abs_difference = std::fabs(pp - score.pp);

            bool increase = pp > score.pp;
            const char *prefix = increase ? "+" : "-";

            if (abs_difference > 1) {
                LOG_F(FILE_ONLY, "Recalculation: Score #{} (user id {}): {}pp -> {}pp ({}{}pp)", score.id, user_id, score.pp, pp, prefix, abs_difference);
            }

            db(update(tables::scores_table).set(tables::scores_table.pp = pp).where(tables::scores_table.id == score.id));
        }
    }

    LOG_F(INFO, "Thread {} finished pp calculation.", naga::get_thread_name());

    // We're done, let the end callback know that
    end(mode, is_relax);
}
