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

#include "../beatmaps/beatmap.hh"
#include "../beatmaps/beatmap_helper.hh"
#include "../config/score_submission_file.hh"
#include "../database/tables/score_table.hh"
#include "../geoloc/country_ids.hh"
#include "../thirdparty/naga.hh"
#include "../thirdparty/oppai.hh"
#include "../users/user_manager.hh"
#include "../users/user_punishments.hh"
#include "../utils/mods.hh"
#include "score_helper.hh"

// Internal function to sort arrays

bool score_sorting(const shiro::scores::score& s_left, const shiro::scores::score& s_right) {
    return s_left.total_score > s_right.total_score;
};

bool pp_sorting(const shiro::scores::score& s_left, const shiro::scores::score& s_right) {
    return s_left.pp > s_right.pp;
};

// End of internal functions

#undef ERROR

shiro::scores::score shiro::scores::helper::fetch_top_score_user(std::string beatmap_md5sum, std::shared_ptr<shiro::users::user> user, bool is_relax) {
    auto db = shiro::database::instance->pop();

    auto result = db(select(all_of(tables::scores_table)).from(tables::scores_table).where(
        tables::scores_table.beatmap_md5 == beatmap_md5sum and
        tables::scores_table.user_id == user->user_id and
        tables::scores_table.is_relax == is_relax and
        tables::scores_table.completed == true
    ));

    if (result.empty()) {
        return score(-1);
    }

    std::vector<score> scores;

    for (const auto &row : result) {
        score s;

        s.id = row.id;
        s.user_id = row.user_id;
        s.hash = row.hash;
        s.beatmap_md5 = beatmap_md5sum;

        s.rank = row.ranking;
        s.total_score = row.score;
        s.max_combo = row.max_combo;
        s.pp = row.pp;

        s.accuracy = row.accuracy;
        s.mods = row.mods;

        s.fc = row.full_combo;
        s.passed = true;
        s.is_relax = is_relax;

        s.count_300 = row.count_300;
        s.count_100 = row.count_100;
        s.count_50 = row.count_50;
        s.count_katus = row.count_katus;
        s.count_gekis = row.count_gekis;
        s.count_misses = row.count_misses;

        s.play_mode = row.play_mode;
        s.time = row.time;

        if (!is_ranked(s, beatmaps::beatmap())) {
            continue;
        }

        scores.emplace_back(s);
    }

    if (scores.empty()) {
        return score(-1);
    }

    std::sort(scores.begin(), scores.end(), (is_relax ? pp_sorting : score_sorting));

    return scores.at(0);
}

std::vector<shiro::scores::score> shiro::scores::helper::fetch_all_scores(std::string beatmap_md5sum, bool is_relax, size_t limit) {
    auto db = shiro::database::instance->pop();

    auto result = db(select(all_of(tables::scores_table)).from(tables::scores_table).where(
        tables::scores_table.beatmap_md5 == beatmap_md5sum and
        tables::scores_table.is_relax == is_relax and
        tables::scores_table.completed == true
    ));

    if (result.empty()) {
        return {};
    }

    beatmaps::beatmap map;
    map.beatmap_md5 = beatmap_md5sum;

    if (!map.fetch_db()) {
        LOG_F(ERROR, "Tried to fetch scores for beatmap hash {} without it being in database.", beatmap_md5sum);
        return {};
    }

    std::vector<score> scores;

    for (const auto &row : result) {
        score s;

        s.id = row.id;
        s.user_id = row.user_id;
        s.hash = row.hash;
        s.beatmap_md5 = beatmap_md5sum;

        s.rank = row.ranking;
        s.total_score = row.score;
        s.max_combo = row.max_combo;
        s.pp = row.pp;

        s.accuracy = row.accuracy;
        s.mods = row.mods;

        s.fc = row.full_combo;
        s.passed = true;
        s.is_relax = is_relax;

        s.count_300 = row.count_300;
        s.count_100 = row.count_100;
        s.count_50 = row.count_50;
        s.count_katus = row.count_katus;
        s.count_gekis = row.count_gekis;
        s.count_misses = row.count_misses;

        s.play_mode = row.play_mode;
        s.time = row.time;

        if (!users::punishments::has_scores(s.user_id)) {
            continue;
        }

        if (!is_ranked(s, beatmaps::beatmap())) {
            continue;
        }

        if (map.last_update > s.time) {
            continue;
        }

        scores.emplace_back(s);
    }

    scores.erase(std::remove_if(scores.begin(), scores.end(), [&](const score &s) {
        for (const score &score : scores) {
            if (score.hash == s.hash) {
                continue;
            }

            if (score.user_id == s.user_id && score.total_score > s.total_score) {
                return true;
            }
        }

        return false;
    }), scores.end());

    std::sort(scores.begin(), scores.end(), (is_relax ? pp_sorting : score_sorting));

    if (scores.size() > limit) {
        scores.resize(limit);
    }

    return scores;
}

std::vector<shiro::scores::score> shiro::scores::helper::fetch_country_scores(std::string beatmap_md5sum, uint8_t country, bool is_relax, size_t limit) {
    auto db = shiro::database::instance->pop();

    auto result = db(select(all_of(tables::scores_table)).from(tables::scores_table).where(
        tables::scores_table.beatmap_md5 == beatmap_md5sum and
        tables::scores_table.is_relax == is_relax and
        tables::scores_table.completed == true
    ));

    if (result.empty()) {
        return {};
    }

    beatmaps::beatmap map;
    map.beatmap_md5 = beatmap_md5sum;

    if (!map.fetch_db()) {
        LOG_F(ERROR, "Tried to fetch scores for beatmap hash {} without it being in database.", beatmap_md5sum);
        return {};
    }

    std::vector<score> scores;

    for (const auto &row : result) {
        score s;

        s.id = row.id;
        s.user_id = row.user_id;
        s.hash = row.hash;
        s.beatmap_md5 = beatmap_md5sum;

        s.rank = row.ranking;
        s.total_score = row.score;
        s.max_combo = row.max_combo;
        s.pp = row.pp;

        s.accuracy = row.accuracy;
        s.mods = row.mods;

        s.fc = row.full_combo;
        s.passed = true;
        s.is_relax = is_relax;

        s.count_300 = row.count_300;
        s.count_100 = row.count_100;
        s.count_50 = row.count_50;
        s.count_katus = row.count_katus;
        s.count_gekis = row.count_gekis;
        s.count_misses = row.count_misses;

        s.play_mode = row.play_mode;
        s.time = row.time;

        if (!users::punishments::has_scores(s.user_id)) {
            continue;
        }

        if (!is_ranked(s, beatmaps::beatmap())) {
            continue;
        }

        if (map.last_update > s.time) {
            continue;
        }

        scores.emplace_back(s);
    }

    scores.erase(std::remove_if(scores.begin(), scores.end(), [&](const score &s) {
        std::shared_ptr<users::user> user = users::manager::get_user_by_id(s.user_id);

        if (user == nullptr) {
            user = std::make_shared<users::user>(s.user_id);

            if (!user->init()) {
                return true;
            }
        }

        uint8_t user_country = geoloc::get_country_id(user->country);
        if (user_country != country) {
            return true;
        }

        for (const score &score : scores) {
            if (s.hash == score.hash || s.user_id != score.user_id) {
                continue;
            }

            if (score.total_score > s.total_score) {
                return true;
            }
        }

        return false;
    }), scores.end());

    std::sort(scores.begin(), scores.end(), (is_relax ? pp_sorting : score_sorting));

    if (scores.size() > limit) {
        scores.resize(limit);
    }

    return scores;
}

std::vector<shiro::scores::score> shiro::scores::helper::fetch_mod_scores(std::string beatmap_md5sum, int32_t mods, bool is_relax, size_t limit) {
    auto db = shiro::database::instance->pop();

    auto result = db(select(all_of(tables::scores_table)).from(tables::scores_table).where(
        tables::scores_table.beatmap_md5 == beatmap_md5sum and 
        tables::scores_table.is_relax == is_relax and
        tables::scores_table.completed == true
    ));

    if (result.empty()) {
        return {};
    }

    beatmaps::beatmap map;
    map.beatmap_md5 = beatmap_md5sum;

    if (!map.fetch_db()) {
        LOG_F(ERROR, "Tried to fetch scores for beatmap hash {} without it being in database.", beatmap_md5sum);
        return {};
    }

    std::vector<score> scores;

    for (const auto &row : result) {
        score s;

        s.id = row.id;
        s.user_id = row.user_id;
        s.hash = row.hash;
        s.beatmap_md5 = beatmap_md5sum;

        s.rank = row.ranking;
        s.total_score = row.score;
        s.max_combo = row.max_combo;
        s.pp = row.pp;

        s.accuracy = row.accuracy;
        s.mods = row.mods;

        s.fc = row.full_combo;
        s.passed = true;
        s.is_relax = is_relax;

        s.count_300 = row.count_300;
        s.count_100 = row.count_100;
        s.count_50 = row.count_50;
        s.count_katus = row.count_katus;
        s.count_gekis = row.count_gekis;
        s.count_misses = row.count_misses;

        s.play_mode = row.play_mode;
        s.time = row.time;

        if (!users::punishments::has_scores(s.user_id)) {
            continue;
        }

        if (!is_ranked(s, beatmaps::beatmap())) {
            continue;
        }

        if (map.last_update > s.time) {
            continue;
        }

        scores.emplace_back(s);
    }

    scores.erase(std::remove_if(scores.begin(), scores.end(), [&](const score &s) {
        if (s.mods != mods) {
            return true;
        }

        for (const score &score : scores) {
            if (s.hash == score.hash || s.user_id != score.user_id) {
                continue;
            }

            if (score.total_score > s.total_score) {
                return true;
            }
        }

        return false;
    }), scores.end());

    std::sort(scores.begin(), scores.end(), (is_relax ? pp_sorting : score_sorting));

    if (scores.size() > limit) {
        scores.resize(limit);
    }

    return scores;
}

std::vector<shiro::scores::score> shiro::scores::helper::fetch_friend_scores(std::string beatmap_md5sum, std::shared_ptr<shiro::users::user> user, bool is_relax, size_t limit) {
    auto db = shiro::database::instance->pop();

    auto result = db(select(all_of(tables::scores_table)).from(tables::scores_table).where(
        tables::scores_table.beatmap_md5 == beatmap_md5sum and 
        tables::scores_table.is_relax == is_relax and
        tables::scores_table.completed == true
    ));

    if (result.empty()) {
        return {};
    }

    beatmaps::beatmap map;
    map.beatmap_md5 = beatmap_md5sum;

    if (!map.fetch_db()) {
        LOG_F(ERROR, "Tried to fetch scores for beatmap hash {} without it being in database.", beatmap_md5sum);
        return {};
    }

    std::vector<score> scores;

    for (const auto &row : result) {
        score s;

        s.id = row.id;
        s.user_id = row.user_id;
        s.hash = row.hash;
        s.beatmap_md5 = beatmap_md5sum;

        s.rank = row.ranking;
        s.total_score = row.score;
        s.max_combo = row.max_combo;
        s.pp = row.pp;

        s.accuracy = row.accuracy;
        s.mods = row.mods;

        s.fc = row.full_combo;
        s.passed = true;
        s.is_relax = is_relax;

        s.count_300 = row.count_300;
        s.count_100 = row.count_100;
        s.count_50 = row.count_50;
        s.count_katus = row.count_katus;
        s.count_gekis = row.count_gekis;
        s.count_misses = row.count_misses;

        s.play_mode = row.play_mode;
        s.time = row.time;

        if (!users::punishments::has_scores(s.user_id)) {
            continue;
        }

        if (!is_ranked(s, beatmaps::beatmap())) {
            continue;
        }

        if (map.last_update > s.time) {
            continue;
        }

        scores.emplace_back(s);
    }

    scores.erase(std::remove_if(scores.begin(), scores.end(), [&](const score &s) {
        std::shared_ptr<users::user> score_user = users::manager::get_user_by_id(s.user_id);

        if (score_user == nullptr) {
            score_user = std::make_shared<users::user>(s.user_id);

            if (!score_user->init()) {
                return true;
            }
        }

        if (score_user->user_id == user->user_id) {
            return false;
        }

        return std::find(user->friends.begin(), user->friends.end(), score_user->user_id) == user->friends.end();
    }), scores.end());

    scores.erase(std::remove_if(scores.begin(), scores.end(), [&](const score &s) {
        for (const score &score : scores) {
            if (score.hash == s.hash || score.user_id != s.user_id) {
                continue;
            }

            if (score.total_score > s.total_score) {
                return true;
            }
        }

        return false;
    }), scores.end());

    std::sort(scores.begin(), scores.end(), (is_relax ? pp_sorting : score_sorting));

    if (scores.size() > limit) {
        scores.resize(limit);
    }

    return scores;
}

std::vector<shiro::scores::score> shiro::scores::helper::fetch_user_scores(std::string beatmap_md5sum, std::shared_ptr<shiro::users::user> user, bool is_relax, size_t limit) {
    auto db = shiro::database::instance->pop();

    auto result = db(select(all_of(tables::scores_table)).from(tables::scores_table).where(
        tables::scores_table.beatmap_md5 == beatmap_md5sum and 
        tables::scores_table.user_id == user->user_id and 
        tables::scores_table.is_relax == is_relax and
        tables::scores_table.completed == true
    ));

    if (result.empty()) {
        return {};
    }

    beatmaps::beatmap map;
    map.beatmap_md5 = beatmap_md5sum;

    if (!map.fetch_db()) {
        LOG_F(ERROR, "Tried to fetch scores for beatmap hash {} without it being in database.", beatmap_md5sum);
        return {};
    }

    std::vector<score> scores;

    for (const auto &row : result) {
        score s;

        s.id = row.id;
        s.user_id = row.user_id;
        s.hash = row.hash;
        s.beatmap_md5 = beatmap_md5sum;

        s.rank = row.ranking;
        s.total_score = row.score;
        s.max_combo = row.max_combo;
        s.pp = row.pp;

        s.accuracy = row.accuracy;
        s.mods = row.mods;

        s.fc = row.full_combo;
        s.passed = true;
        s.is_relax = is_relax;

        s.count_300 = row.count_300;
        s.count_100 = row.count_100;
        s.count_50 = row.count_50;
        s.count_katus = row.count_katus;
        s.count_gekis = row.count_gekis;
        s.count_misses = row.count_misses;

        s.play_mode = row.play_mode;
        s.time = row.time;

        if (!is_ranked(s, beatmaps::beatmap())) {
            continue;
        }

        if (map.last_update > s.time) {
            continue;
        }

        scores.emplace_back(s);
    }

    std::sort(scores.begin(), scores.end(), (is_relax ? pp_sorting : score_sorting));

    if (scores.size() > limit) {
        scores.resize(limit);
    }

    return scores;
}

std::vector<shiro::scores::score> shiro::scores::helper::fetch_all_user_scores(int32_t user_id, bool is_relax, size_t limit) {
    auto db = shiro::database::instance->pop();

    auto result = db(select(all_of(tables::scores_table)).from(tables::scores_table).where(
        tables::scores_table.user_id == user_id and 
        tables::scores_table.is_relax == is_relax
    ));

    if (result.empty()) {
        return {};
    }

    std::vector<score> scores;

    for (const auto &row : result) {
        score s;

        s.id = row.id;
        s.user_id = row.user_id;
        s.hash = row.hash;
        s.beatmap_md5 = row.beatmap_md5;

        s.rank = row.ranking;
        s.total_score = row.score;
        s.max_combo = row.max_combo;
        s.pp = row.pp;

        s.accuracy = row.accuracy;
        s.mods = row.mods;

        s.fc = row.full_combo;
        s.passed = row.completed;
        s.is_relax = is_relax;

        s.count_300 = row.count_300;
        s.count_100 = row.count_100;
        s.count_50 = row.count_50;
        s.count_katus = row.count_katus;
        s.count_gekis = row.count_gekis;
        s.count_misses = row.count_misses;

        s.play_mode = row.play_mode;
        s.time = row.time;

        scores.emplace_back(s);
    }

    std::sort(scores.begin(), scores.end(), (is_relax ? pp_sorting : score_sorting));

    if (scores.size() > limit) {
        scores.resize(limit);
    }

    return scores;
}

std::vector<shiro::scores::score> shiro::scores::helper::fetch_top100_user(shiro::utils::play_mode mode, int32_t user_id, bool is_relax) {
    auto db = shiro::database::instance->pop();

    auto result = db(select(all_of(tables::scores_table)).from(tables::scores_table).where(
        tables::scores_table.user_id == user_id and 
        tables::scores_table.play_mode == static_cast<uint8_t>(mode) and 
        tables::scores_table.is_relax == is_relax and
        tables::scores_table.completed == true
    ));

    if (result.empty()) {
        return {};
    }

    std::vector<score> scores;

    for (const auto &row : result) {
        score s;

        s.id = row.id;
        s.user_id = row.user_id;
        s.hash = row.hash;
        s.beatmap_md5 = row.beatmap_md5;

        s.rank = row.ranking;
        s.total_score = row.score;
        s.max_combo = row.max_combo;
        s.pp = row.pp;

        s.accuracy = row.accuracy;
        s.mods = row.mods;

        s.fc = row.full_combo;
        s.passed = true;
        s.is_relax = row.is_relax;

        s.count_300 = row.count_300;
        s.count_100 = row.count_100;
        s.count_50 = row.count_50;
        s.count_katus = row.count_katus;
        s.count_gekis = row.count_gekis;
        s.count_misses = row.count_misses;

        s.play_mode = row.play_mode;
        s.time = row.time;

        if (!is_ranked(s, beatmaps::beatmap())) {
            continue;
        }

        scores.emplace_back(s);
    }

    scores.erase(std::remove_if(scores.begin(), scores.end(), [&](const score &s) {
        beatmaps::beatmap beatmap;
        beatmap.beatmap_md5 = s.beatmap_md5;

        // How do we have a score on the beatmap without having the beatmap in the db?
        if (!beatmap.fetch_db()) {
            LOG_F(ERROR, "Found score for user {} without having beatmap hash {} in database.", user_id, s.beatmap_md5);
            return true;
        }

        if (!beatmaps::helper::has_leaderboard(beatmaps::helper::fix_beatmap_status(beatmap.ranked_status))) {
            return true;
        }

        for (const score &score : scores) {
            if (s.hash == score.hash || s.beatmap_md5 != score.beatmap_md5) {
                continue;
            }

            if (score.total_score > s.total_score) {
                return true;
            }
        }

        return false;
    }), scores.end());

    std::sort(scores.begin(), scores.end(), pp_sorting);

    if (scores.size() > 100) {
        scores.resize(100);
    }

    return scores;
}

std::optional<shiro::scores::score> shiro::scores::helper::get_latest_score(int32_t user_id, const utils::play_mode &mode, bool is_relax) {
    auto db = shiro::database::instance->pop();

    auto result = db(select(all_of(tables::scores_table)).from(tables::scores_table).where(
        tables::scores_table.user_id == user_id and
        tables::scores_table.play_mode == static_cast<uint8_t>(mode) and
        tables::scores_table.is_relax == is_relax
    ).order_by(tables::scores_table.time.desc()).limit(1u));

    if (result.empty()) {
        return std::nullopt;
    }

    // We are ordering by time (descending) so the front value will have the highest timestamp which is the latest in unix time
    const auto &row = result.front();

    score s(-1);

    s.id = row.id;
    s.user_id = row.user_id;
    s.hash = row.hash;
    s.beatmap_md5 = row.beatmap_md5;

    s.rank = row.ranking;
    s.total_score = row.score;
    s.max_combo = row.max_combo;
    s.pp = row.pp;

    s.accuracy = row.accuracy;
    s.mods = row.mods;

    s.fc = row.full_combo;
    s.passed = row.completed;
    s.is_relax = is_relax;

    s.count_300 = row.count_300;
    s.count_100 = row.count_100;
    s.count_50 = row.count_50;
    s.count_katus = row.count_katus;
    s.count_gekis = row.count_gekis;
    s.count_misses = row.count_misses;

    s.play_mode = row.play_mode;
    s.time = row.time;

    return s;
}

shiro::scores::score shiro::scores::helper::get_score(int32_t id) {
    auto db = shiro::database::instance->pop();

    auto result = db(select(all_of(tables::scores_table)).from(tables::scores_table).where(tables::scores_table.id == id).limit(1u));

    if (result.empty()) {
        return score(-1);
    }

    const auto &row = result.front();
    score s(-1);

    s.id = row.id;
    s.user_id = row.user_id;
    s.hash = row.hash;
    s.beatmap_md5 = row.beatmap_md5;

    s.rank = row.ranking;
    s.total_score = row.score;
    s.max_combo = row.max_combo;
    s.pp = row.pp;

    s.accuracy = row.accuracy;
    s.mods = row.mods;

    s.fc = row.full_combo;
    s.passed = row.completed;
    s.is_relax = row.is_relax;

    s.count_300 = row.count_300;
    s.count_100 = row.count_100;
    s.count_50 = row.count_50;
    s.count_katus = row.count_katus;
    s.count_gekis = row.count_gekis;
    s.count_misses = row.count_misses;

    s.play_mode = row.play_mode;
    s.time = row.time;

    return s;
}

int32_t shiro::scores::helper::get_scoreboard_position(const shiro::scores::score &s, std::vector<score> &scores) {
    std::sort(scores.begin(), scores.end(), (s.is_relax ? pp_sorting : score_sorting));

    for (size_t i = 0; i < scores.size(); i++) {
        score &beatmap_score = scores.at(i);

        if (beatmap_score.id == s.id) {
            return i + 1;
        }
    }

    return -1;
}

bool shiro::scores::helper::is_ranked(const shiro::scores::score &score, const shiro::beatmaps::beatmap &beatmap) {
    bool ranked = true;
    int32_t mods = score.mods;

    switch (static_cast<utils::play_mode>(score.play_mode)) {
        case utils::play_mode::standard: {
            ranked &= config::score_submission::std_ranked;
            break;
        }
        case utils::play_mode::taiko: {
            ranked &= config::score_submission::taiko_ranked;
            break;
        }
        case utils::play_mode::fruits: {
            ranked &= config::score_submission::catch_ranked;
            break;
        }
        case utils::play_mode::mania: {
            ranked &= config::score_submission::mania_ranked;
            break;
        }
    }

    if (mods & static_cast<int32_t>(utils::mods::none)) {
        ranked &= config::score_submission::nomod_ranked;
    }

    if (mods & static_cast<int32_t>(utils::mods::no_fail)) {
        ranked &= config::score_submission::no_fail_ranked;
    }

    if (mods & static_cast<int32_t>(utils::mods::easy)) {
        ranked &= config::score_submission::easy_ranked;
    }

    if (mods & static_cast<int32_t>(utils::mods::touch_device)) {
        ranked &= config::score_submission::touch_device_ranked;
    }

    if (mods & static_cast<int32_t>(utils::mods::hidden)) {
        ranked &= config::score_submission::hidden_ranked;
    }

    if (mods & static_cast<int32_t>(utils::mods::hard_rock)) {
        ranked &= config::score_submission::hard_rock_ranked;
    }

    if (mods & static_cast<int32_t>(utils::mods::sudden_death)) {
        ranked &= config::score_submission::sudden_death_ranked;
    }

    if (mods & static_cast<int32_t>(utils::mods::double_time)) {
        ranked &= config::score_submission::double_time_ranked;
    }

    if (mods & static_cast<int32_t>(utils::mods::relax)) {
        ranked &= config::score_submission::relax_ranked;
    }

    if (mods & static_cast<int32_t>(utils::mods::half_time)) {
        ranked &= config::score_submission::half_time_ranked;
    }

    if (mods & static_cast<int32_t>(utils::mods::nightcore)) {
        ranked &= config::score_submission::nightcore_ranked;
    }

    if (mods & static_cast<int32_t>(utils::mods::flashlight)) {
        ranked &= config::score_submission::flashlight_ranked;
    }

    if (mods & static_cast<int32_t>(utils::mods::auto_play)) {
        ranked &= config::score_submission::auto_play_ranked;
    }

    if (mods & static_cast<int32_t>(utils::mods::spun_out)) {
        ranked &= config::score_submission::spun_out_ranked;
    }

    if (mods & static_cast<int32_t>(utils::mods::auto_pilot)) {
        ranked &= config::score_submission::auto_pilot_ranked;
    }

    if (mods & static_cast<int32_t>(utils::mods::perfect)) {
        ranked &= config::score_submission::perfect_ranked;
    }

    // Special mods

    if (mods & static_cast<int32_t>(utils::mods::fade_in)) {
        ranked &= config::score_submission::fade_in_ranked;
    }

    if (mods & static_cast<int32_t>(utils::mods::random)) {
        ranked &= config::score_submission::random_ranked;
    }

    if (mods & static_cast<int32_t>(utils::mods::cinema)) {
        ranked &= config::score_submission::cinema_ranked;
    }

    if (mods & static_cast<int32_t>(utils::mods::target)) {
        ranked &= config::score_submission::target_ranked;
    }

    if (mods & static_cast<int32_t>(utils::mods::score_v2)) {
        ranked &= config::score_submission::score_v2_ranked;
    }

    // Keys

    if (mods & static_cast<int32_t>(utils::mods::key_1)) {
        ranked &= config::score_submission::key_1_ranked;
    }

    if (mods & static_cast<int32_t>(utils::mods::key_2)) {
        ranked &= config::score_submission::key_2_ranked;
    }

    if (mods & static_cast<int32_t>(utils::mods::key_3)) {
        ranked &= config::score_submission::key_3_ranked;
    }

    if (mods & static_cast<int32_t>(utils::mods::key_4)) {
        ranked &= config::score_submission::key_4_ranked;
    }

    if (mods & static_cast<int32_t>(utils::mods::key_5)) {
        ranked &= config::score_submission::key_5_ranked;
    }

    if (mods & static_cast<int32_t>(utils::mods::key_6)) {
        ranked &= config::score_submission::key_6_ranked;
    }

    if (mods & static_cast<int32_t>(utils::mods::key_7)) {
        ranked &= config::score_submission::key_7_ranked;
    }

    if (mods & static_cast<int32_t>(utils::mods::key_8)) {
        ranked &= config::score_submission::key_8_ranked;
    }

    if (mods & static_cast<int32_t>(utils::mods::key_9)) {
        ranked &= config::score_submission::key_9_ranked;
    }

    if (mods & static_cast<int32_t>(utils::mods::key_coop)) {
        ranked &= config::score_submission::key_coop_ranked;
    }

    if (beatmap.id != 0) {
        ranked &= beatmaps::helper::has_leaderboard(beatmaps::helper::fix_beatmap_status(beatmap.ranked_status));
    }

    return ranked;
}

std::tuple<bool, std::string> shiro::scores::helper::is_flagged(const shiro::scores::score &score, const shiro::beatmaps::beatmap &beatmap) {
    if (!is_ranked(score, beatmap) || !score.passed) {
        return { false, "" };
    }

    if (config::score_submission::restrict_negative_values) {
        if (score.total_score < 0) {
            return { true, "Negative score value (total score " + std::to_string(score.total_score) + " < 0)" };
        }

        if (score.max_combo < 0) {
            return { true, "Negative score value (max combo " + std::to_string(score.max_combo) + " < 0)" };
        }

        if (score.count_300 < 0) {
            return { true, "Negative score value (300 count " + std::to_string(score.count_300) + " < 0)" };
        }

        if (score.count_100 < 0) {
            return { true, "Negative score value (100 count " + std::to_string(score.count_100) + " < 0)" };
        }

        if (score.count_50 < 0) {
            return { true, "Negative score value (50 count " + std::to_string(score.count_50) + " < 0)" };
        }

        if (score.count_katus < 0) {
            return { true, "Negative score value (katus count " + std::to_string(score.count_katus) + " < 0)" };
        }

        if (score.count_gekis < 0) {
            return { true, "Negative score value (gekis count " + std::to_string(score.count_gekis) + " < 0)" };
        }

        if (score.count_misses < 0) {
            return { true, "Negative score value (miss count " + std::to_string(score.count_misses) + " < 0)" };
        }

        if (score.mods < 0) {
            return { true, "Negative score value (mods " + std::to_string(score.mods) + " < 0)" };
        }
    }

    if (config::score_submission::restrict_impossible_mods) {
        int32_t mods = score.mods;

        if (mods & static_cast<int32_t>(utils::mods::easy) && mods & static_cast<int32_t>(utils::mods::hard_rock)) {
            return { true, "Impossible mod combination (EZ + HR)" };
        }

        if (mods & static_cast<int32_t>(utils::mods::no_fail) && mods & static_cast<int32_t>(utils::mods::sudden_death)) {
            return { true, "Impossible mod combination (NF + SD)" };
        }

        if (mods & static_cast<int32_t>(utils::mods::no_fail) && mods & static_cast<int32_t>(utils::mods::perfect)) {
            return { true, "Impossible mod combination (NF + PF)" };
        }

        if (mods & static_cast<int32_t>(utils::mods::half_time) && mods & static_cast<int32_t>(utils::mods::double_time)) {
            return { true, "Impossible mod combination (HT + DT)" };
        }

        if (mods & static_cast<int32_t>(utils::mods::sudden_death) && mods & static_cast<int32_t>(utils::mods::relax)) {
            return { true, "Impossible mod combination (SD + RX)" };
        }

        if (mods & static_cast<int32_t>(utils::mods::sudden_death) && mods & static_cast<int32_t>(utils::mods::auto_pilot)) {
            return { true, "Impossible mod combination (SD + AP)" };
        }

        if (mods & static_cast<int32_t>(utils::mods::sudden_death) && mods & static_cast<int32_t>(utils::mods::auto_play)) {
            return { true, "Impossible mod combination (SD + AUTO)" };
        }

        if (mods & static_cast<int32_t>(utils::mods::relax) && mods & static_cast<int32_t>(utils::mods::no_fail)) {
            return { true, "Impossible mod combination (RX + NF)" };
        }

        if (mods & static_cast<int32_t>(utils::mods::relax) && mods & static_cast<int32_t>(utils::mods::auto_pilot)) {
            return { true, "Impossible mod combination (RX + AP)" };
        }

        if (mods & static_cast<int32_t>(utils::mods::relax) && mods & static_cast<int32_t>(utils::mods::auto_play)) {
            return { true, "Impossible mod combination (RX + AUTO)" };
        }

        if (mods & static_cast<int32_t>(utils::mods::auto_pilot) && mods & static_cast<int32_t>(utils::mods::no_fail)) {
            return { true, "Impossible mod combination (AP + NF)" };
        }

        if (mods & static_cast<int32_t>(utils::mods::auto_pilot) && mods & static_cast<int32_t>(utils::mods::spun_out)) {
            return { true, "Impossible mod combination (AP + SO)" };
        }

        if (mods & static_cast<int32_t>(utils::mods::auto_pilot) && mods & static_cast<int32_t>(utils::mods::auto_play)) {
            return { true, "Impossible mod combination (AP + AUTO)" };
        }

        if (mods & static_cast<int32_t>(utils::mods::spun_out) && mods & static_cast<int32_t>(utils::mods::auto_play)) {
            return { true, "Impossible mod combination (SO + AUTO)" };
        }
    }

    bool impossible_combo = 
            score.play_mode == static_cast<uint8_t>(utils::play_mode::standard) &&
            beatmap.max_combo > 0 &&
            score.max_combo > beatmap.max_combo;

    if (config::score_submission::restrict_impossible_combo && impossible_combo) {
        return { true, "Impossible combo (" + std::to_string(score.max_combo) + " > " + std::to_string(beatmap.max_combo) + ")" };
    }

    if (score.is_relax) {
        switch (static_cast<utils::play_mode>(score.play_mode)) {
            case utils::play_mode::standard: {
                if (config::score_submission::auto_restrict_pp_std_relax > -1 && score.pp > config::score_submission::auto_restrict_pp_std_relax) {
                    return { true, "High pp gain (osu!std - Relax, " + std::to_string(score.pp) + "pp > " + std::to_string(config::score_submission::auto_restrict_pp_std_relax) + "pp)" };
                }
                break;
            }
            case utils::play_mode::taiko: {
                if (config::score_submission::auto_restrict_pp_taiko_relax > -1 && score.pp > config::score_submission::auto_restrict_pp_taiko_relax) {
                    return { true, "High pp gain (osu!taiko - Relax, " + std::to_string(score.pp) + "pp > " + std::to_string(config::score_submission::auto_restrict_pp_taiko_relax) + "pp)" };
                }
                break;
            }
            case utils::play_mode::fruits: {
                if (config::score_submission::auto_restrict_pp_catch_relax > -1 && score.pp > config::score_submission::auto_restrict_pp_catch_relax) {
                    return { true, "High pp gain (osu!catch - Relax, " + std::to_string(score.pp) + "pp > " + std::to_string(config::score_submission::auto_restrict_pp_catch_relax) + "pp)" };
                }
                break;
            }
            case utils::play_mode::mania: {
                return { true, "Impossible mode and mod combination (osu!mania + Relax)" };
            }
        }
    } 
    else {
        switch (static_cast<utils::play_mode>(score.play_mode)) {
            case utils::play_mode::standard: {
                if (config::score_submission::auto_restrict_pp_std > -1 && score.pp > config::score_submission::auto_restrict_pp_std) {
                    return { true, "High pp gain (osu!std, " + std::to_string(score.pp) + "pp > " + std::to_string(config::score_submission::auto_restrict_pp_std) + "pp)" };
                }
                break;
            }
            case utils::play_mode::taiko: {
                if (config::score_submission::auto_restrict_pp_taiko > -1 && score.pp > config::score_submission::auto_restrict_pp_taiko) {
                    return { true, "High pp gain (osu!taiko, " + std::to_string(score.pp) + "pp > " + std::to_string(config::score_submission::auto_restrict_pp_taiko) + "pp)" };
                }
                break;
            }
            case utils::play_mode::fruits: {
                if (config::score_submission::auto_restrict_pp_catch > -1 && score.pp > config::score_submission::auto_restrict_pp_catch) {
                    return { true, "High pp gain (osu!catch, " + std::to_string(score.pp) + "pp > " + std::to_string(config::score_submission::auto_restrict_pp_catch) + "pp)" };
                }
                break;
            }
            case utils::play_mode::mania: {
                if (config::score_submission::auto_restrict_pp_mania > -1 && score.pp > config::score_submission::auto_restrict_pp_mania) {
                    return { true, "High pp gain (osu!mania, " + std::to_string(score.pp) + "pp > " + std::to_string(config::score_submission::auto_restrict_pp_mania) + "pp)" };
                }
                break;
            }
        }
    }

    return { false, "" };
}

float shiro::scores::helper::calculate_accuracy(utils::play_mode mode, int32_t _300, int32_t _100, int32_t _50, int32_t geki, int32_t katu, int32_t miss) {
    switch (mode) {
        case utils::play_mode::standard: {
            int32_t total_hits = _300 + _100 + _50 + miss;

            if (total_hits <= 0) {
                return 0.0f;
            }

            float accuracy = (_300 * 300.0f + _100 * 100.0f + _50 * 50.0f) / (total_hits * 300.0f);
            return accuracy * 100;
        }
        case utils::play_mode::taiko: {
            int32_t _150 = _100 + _50;
            int32_t total_hits = _300 + _150 + miss;

            if (total_hits <= 0) {
                return 0.0f;
            }

            float accuracy = (_300 * 300.0f + _150 * 150.0f) / (total_hits * 300.0f);
            return accuracy * 100;
        }
        case utils::play_mode::fruits: {
            int32_t total = _50 + _100 + _300 + katu + miss;

            if (total <= 0) {
                return 0.0;
            }

            int32_t numerator = _50 + _100 + _300;
            float denominator = total;

            return (numerator / denominator) * 100;
        }
        case utils::play_mode::mania: {
            int32_t total = _50 + _100 + katu + _300 + geki + miss;

            if (total <= 0) {
                return 0.0;
            }

            int32_t numerator = 50 * _50 + 100 * _100 + 200 * katu + 300 * (_300 + geki);
            float denominator = 300 * total;

            return (numerator / denominator) * 100;
        }
    }

    return 0.0f;
}

std::string shiro::scores::helper::build_mods_list(int32_t mods)
{
    std::string modString = "+";

    if (mods & static_cast<int32_t>(shiro::utils::mods::no_fail)) {
        modString += "NF";
    }

    if (mods & static_cast<int32_t>(shiro::utils::mods::easy)) {
        modString += "EZ";
    }

    if (mods & static_cast<int32_t>(shiro::utils::mods::half_time)) {
        modString += "HT";
    }

    if (mods & static_cast<int32_t>(shiro::utils::mods::hidden)) {
        modString += "HD";
    }

    if (mods & static_cast<int32_t>(shiro::utils::mods::hard_rock)) {
        modString += "HR";
    }

    if (mods & static_cast<int32_t>(shiro::utils::mods::double_time)) {
        modString += "DT";
    }

    if (mods & static_cast<int32_t>(shiro::utils::mods::nightcore)) {
        modString += "NC";
    }

    if (mods & static_cast<int32_t>(shiro::utils::mods::flashlight)) {
        modString += "FL";
    }

    if (mods & static_cast<int32_t>(shiro::utils::mods::sudden_death)) {
        modString += "SD";
    }

    if (mods & static_cast<int32_t>(shiro::utils::mods::perfect)) {
        modString += "PF";
    }

    if (mods & static_cast<int32_t>(shiro::utils::mods::relax)) {
        modString += "RX";
    }

    if (mods & static_cast<int32_t>(shiro::utils::mods::auto_pilot)) {
        modString += "AT";
    }

    if (mods & static_cast<int32_t>(shiro::utils::mods::spun_out)) {
        modString += "SO";
    }


    if (modString == "+") {
        modString = "No Mod";
    }

    return modString;
}
