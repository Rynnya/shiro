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

#include <chrono>
#include <iomanip>

#include "../config/bancho_file.hh"
#include "../config/ipc_file.hh"
#include "../database/tables/beatmap_table.hh"
#include "../logger/sentry_logger.hh"
#include "../thirdparty/json.hh"
#include "../thirdparty/naga.hh"
#include "../utils/curler.hh"
#include "../utils/string_utils.hh"
#include "../shiro.hh"
#include "beatmap.hh"
#include "beatmap_helper.hh"

void shiro::beatmaps::beatmap::fetch(bool force_peppster) {
    if (this->beatmapset_id == -1) {
        this->ranked_status = static_cast<int32_t>(status::unsubmitted);
        return;
    }

    if (!force_peppster) {
        if (fetch_db()) {
            return;
        }
    }

    if (!fetch_api()) {
        this->ranked_status = static_cast<int32_t>(status::unknown);
    }
}

bool shiro::beatmaps::beatmap::fetch_db() {
    auto db = shiro::database::instance->pop();

    auto result = db(select(all_of(tables::beatmaps_table)).from(tables::beatmaps_table).where(tables::beatmaps_table.beatmap_md5 == this->beatmap_md5).limit(1u));

    if (result.empty()) {
        return false;
    }

    const auto& row = result.front();

    this->id = row.id;
    this->beatmap_id = row.beatmap_id;
    this->beatmapset_id = row.beatmapset_id;
    this->play_mode = row.mode;
    this->beatmap_md5 = row.beatmap_md5;
    this->artist = row.artist;
    this->title = row.title;
    this->difficulty_name = row.difficulty_name;
    this->creator = row.creator;

    this->song_name = fmt::format("{} - {} [{}]", this->artist, this->title, this->difficulty_name);;

    this->cs = row.cs;
    this->ar = row.ar;
    this->od = row.od;
    this->hp = row.hp;
    this->difficulty_std = row.difficulty_std;
    this->difficulty_taiko = row.difficulty_taiko;
    this->difficulty_ctb = row.difficulty_ctb;
    this->difficulty_mania = row.difficulty_mania;
    this->max_combo = row.max_combo;
    this->hit_length = row.hit_length;
    this->bpm = row.bpm;
    this->count_normal = row.count_normal;
    this->count_slider = row.count_slider;
    this->count_spinner = row.count_spinner;
    this->ranked_status = row.ranked_status;
    this->ranked_status_freezed = row.ranked_status_freezed;
    this->last_update = row.latest_update;
    this->play_count = row.play_count;
    this->pass_count = row.pass_count;

    return true;
}

bool shiro::beatmaps::beatmap::fetch_api() {
    if (this->beatmapset_id == 0) {
        const std::string url = fmt::format("https://old.ppy.sh/api/get_beatmaps?k={}&b={}", config::bancho::api_key, this->beatmap_id);
        auto [success, output] = utils::curl::get(url);

        if (!success) {
            LOG_F(ERROR, "Unable to connect to osu! api: {}.", output);

            this->ranked_status = static_cast<int32_t>(status::unknown);
            return false;
        }

        json json_result;
        try {
            json_result = json::parse(output);
        }
        catch (const json::parse_error& ex) {
            LOG_F(ERROR, "Unable to parse json response from osu! api.");
            CAPTURE_EXCEPTION(ex);

            this->ranked_status = static_cast<int32_t>(status::unknown);
            return false;
        }

        for (auto& part : json_result) {
            if (!utils::strings::evaluate(part["beatmapset_id"], this->beatmapset_id)) {
                LOG_F(ERROR, "Unable to cast response of osu! API to valid data types: bad cast on beatmapset_id (string -> int32_t)");
                CAPTURE_EXCEPTION(std::invalid_argument("Invalid data type in beatmapset_id"));

                this->ranked_status = static_cast<int32_t>(status::unknown);
                return false;
            }

            this->beatmap_md5 = part["file_md5"];
        }
    }

    const std::string url = fmt::format("https://old.ppy.sh/api/get_beatmaps?k={}&s={}", config::bancho::api_key, this->beatmapset_id);
    auto [success, output] = utils::curl::get(url);

    if (!success) {
        LOG_F(ERROR, "Unable to connect to osu! api: {}.", output);

        this->ranked_status = static_cast<int32_t>(status::unknown);
        return false;
    }

    std::string original_md5sum = this->beatmap_md5;
    bool map_found = false;
    json json_result;

    try {
        json_result = json::parse(output);
    }
    catch (const json::parse_error& ex) {
        LOG_F(ERROR, "Unable to parse json response from osu! api.");
        CAPTURE_EXCEPTION(ex);

        this->ranked_status = static_cast<int32_t>(status::unknown);
        return false;
    }

    /* This way faster than lexical_cast, but provides less information why this thing crashed... */
    /* If you have any problems - please debug program to catch real exception (bad_lexical_cast don't give any information) */
    /* Personally I think that ppy's api cannot crash this in any way */

    for (auto& part : json_result) {
        bool parse_result = true;

        this->artist = part["artist"];
        this->title = part["title"];
        this->difficulty_name = part["version"];
        this->creator = part["creator"];
        this->beatmap_md5 = part["file_md5"];

        this->song_name = fmt::format("{} - {} [{}]", this->artist, this->title, this->difficulty_name);

        parse_result &= utils::strings::evaluate(part["beatmapset_id"], this->beatmapset_id);
        parse_result &= utils::strings::evaluate(part["beatmap_id"],    this->beatmap_id);
        parse_result &= utils::strings::evaluate(part["approved"],      this->ranked_status);
        parse_result &= utils::strings::evaluate(part["hit_length"],    this->hit_length);
        parse_result &= utils::strings::evaluate(part["mode"],          this->play_mode);
        parse_result &= utils::strings::evaluate(part["diff_size"],     this->cs);
        parse_result &= utils::strings::evaluate(part["diff_approach"], this->ar);
        parse_result &= utils::strings::evaluate(part["diff_overall"],  this->od);
        parse_result &= utils::strings::evaluate(part["diff_drain"],    this->hp);
        parse_result &= utils::strings::evaluate(part["bpm"],           this->bpm);
        parse_result &= utils::strings::evaluate(part["count_normal"],  this->count_normal);
        parse_result &= utils::strings::evaluate(part["count_slider"],  this->count_slider);
        parse_result &= utils::strings::evaluate(part["count_spinner"], this->count_spinner);

        switch (static_cast<utils::play_mode>(this->play_mode)) {
            case utils::play_mode::standard: {
                parse_result &= utils::strings::evaluate(part["difficultyrating"], this->difficulty_std);

                // For some older beatmaps the max_combo is null. See ppy/osu-api#130
                if (part["max_combo"].is_string()) {
                    parse_result &= utils::strings::evaluate(part["max_combo"], this->max_combo);
                }

                break;
            }
            case utils::play_mode::taiko: {
                parse_result &= utils::strings::evaluate(part["difficultyrating"], this->difficulty_taiko);
                break;
            }
            case utils::play_mode::fruits: {
                parse_result &= utils::strings::evaluate(part["difficultyrating"], this->difficulty_ctb);

                // For some older beatmaps the max_combo is null. See ppy/osu-api#130
                if (part["max_combo"].is_string()) {
                    parse_result &= utils::strings::evaluate(part["max_combo"], this->max_combo);
                }

                break;
            }
            case utils::play_mode::mania: {
                parse_result &= utils::strings::evaluate(part["difficultyrating"], this->difficulty_mania);
                break;
            }
        }

        // Beatmap is unranked
        if (part["approved_date"].is_null()) {
            this->ranked_status = static_cast<int32_t>(status::latest_pending);
        }

        if (!parse_result) {
            LOG_F(ERROR, "Unable to cast response of Bancho API to valid data types.");
            CAPTURE_EXCEPTION(std::invalid_argument("Invalid data type in beatmapset_id"));

            this->ranked_status = static_cast<int32_t>(status::unknown);
            return false;
        }

        if (this->beatmap_md5 == original_md5sum) {
            map_found = true;
        }

        std::string last_update = part["last_update"];

        std::tm time = std::tm();
        std::stringstream stream = std::stringstream(last_update);

        stream >> std::get_time(&time, "%Y-%m-%d %H:%M:%S");
        std::chrono::time_point time_point = std::chrono::system_clock::from_time_t(std::mktime(&time));
        std::chrono::seconds seconds = std::chrono::time_point_cast<std::chrono::seconds>(time_point).time_since_epoch();

        this->last_update = seconds.count();

        save();
    }

    this->beatmap_md5 = original_md5sum;

    if (!map_found)  {
        // Map was not found when queuing for the beatmap set, the map is not submitted.
        this->ranked_status = static_cast<int32_t>(status::unsubmitted);
        return true;
    }

    fetch_db();
    return true;
}

bool shiro::beatmaps::beatmap::exist() {
    if (beatmapset_id == 0) {
        return false;
    }

    auto db = shiro::database::instance->pop();
    auto result = db(select(all_of(tables::beatmaps_table)).from(tables::beatmaps_table).where(tables::beatmaps_table.beatmapset_id == this->beatmapset_id));

    for (const auto& row : result) {
        if (row.beatmap_id.value() == this->beatmap_id) {
            return true;
        }
    }

    return false;
}

void shiro::beatmaps::beatmap::save() {
    auto db = shiro::database::instance->pop();

    auto result = db(sqlpp::select(tables::beatmaps_table.id).from(tables::beatmaps_table).where(tables::beatmaps_table.beatmap_md5 == this->beatmap_md5).limit(1u));
    if (!result.empty()) {
        return;
    }

    db(insert_into(tables::beatmaps_table).set(
        tables::beatmaps_table.beatmap_id = this->beatmap_id,
        tables::beatmaps_table.beatmapset_id = this->beatmapset_id,
        tables::beatmaps_table.beatmap_md5 = this->beatmap_md5,
        tables::beatmaps_table.artist = this->artist,
        tables::beatmaps_table.title = this->title,
        tables::beatmaps_table.difficulty_name = this->difficulty_name,
        tables::beatmaps_table.creator = this->creator,
        tables::beatmaps_table.cs = this->cs,
        tables::beatmaps_table.ar = this->ar,
        tables::beatmaps_table.od = this->od,
        tables::beatmaps_table.hp = this->hp,
        tables::beatmaps_table.mode = this->play_mode,
        tables::beatmaps_table.difficulty_std = this->difficulty_std,
        tables::beatmaps_table.difficulty_taiko = this->difficulty_taiko,
        tables::beatmaps_table.difficulty_ctb = this->difficulty_ctb,
        tables::beatmaps_table.difficulty_mania = this->difficulty_mania,
        tables::beatmaps_table.max_combo = this->max_combo,
        tables::beatmaps_table.hit_length = this->hit_length,
        tables::beatmaps_table.bpm = this->bpm,
        tables::beatmaps_table.count_normal = this->count_normal,
        tables::beatmaps_table.count_slider = this->count_slider,
        tables::beatmaps_table.count_spinner = this->count_spinner,
        tables::beatmaps_table.ranked_status = this->ranked_status,
        tables::beatmaps_table.ranked_status_freezed = this->ranked_status_freezed,
        tables::beatmaps_table.latest_update = this->last_update,
        tables::beatmaps_table.play_count = this->play_count,
        tables::beatmaps_table.pass_count = this->pass_count
    ));
}

void shiro::beatmaps::beatmap::update_play_metadata() {
    auto db = shiro::database::instance->pop();

    db(update(tables::beatmaps_table).set(
        tables::beatmaps_table.play_count = this->play_count,
        tables::beatmaps_table.pass_count = this->pass_count
    ).where(tables::beatmaps_table.beatmap_md5 == this->beatmap_md5));
}

std::string shiro::beatmaps::beatmap::get_url() {
    return fmt::format("{}{}", config::ipc::beatmap_url, this->beatmap_id);
}

std::string shiro::beatmaps::beatmap::build_header() {
    return fmt::format(
        "{}|false|{}|{}|{}\n0\n{}\n10.0\n",
        helper::fix_beatmap_status(this->ranked_status),
        this->beatmap_id, this->beatmapset_id, this->pass_count,
        this->song_name
    );
}

std::string shiro::beatmaps::beatmap::build_header(const std::vector<scores::score>& scores) {
    return fmt::format(
        "{}|false|{}|{}|{}\n0\n{}\n10.0\n",
        helper::fix_beatmap_status(this->ranked_status),
        this->beatmap_id, this->beatmapset_id, scores.size(),
        this->song_name
    );
}
