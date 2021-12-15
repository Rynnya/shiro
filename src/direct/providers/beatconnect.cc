/*
 * shiro - High performance, high quality osu!Bancho C++ re-implementation
 * Copyright (C) 2018-2020 Marc3842h, czapek
 * Copyright (C) 2021 Rynnya
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

#include "../../beatmaps/beatmap_helper.hh"
#include "../../database/tables/beatmap_table.hh"
#include "../../logger/sentry_logger.hh"
#include "../../thirdparty/loguru.hh"
#include "../../thread/thread_pool.hh"
#include "../../utils/curler.hh"
#include "../../utils/play_mode.hh"
#include "../../utils/string_utils.hh"
#include "../../shiro.hh"
#include "beatconnect.hh"

void shiro::direct::beatconnect::search(crow::response& callback, std::unordered_map<std::string, std::string> parameters) {
    // Remove username from the request so the requesting user stays anonymous
    if (parameters.find("u") != parameters.end()) {
        parameters.erase("u");
    }

    // Remove password hash from the request so no credentials are leaked
    if (parameters.find("h") != parameters.end()) {
        parameters.erase("h");
    }

    std::string url = "https://beatconnect.io/api/search/?";

    for (const auto &[key, value] : parameters) {
        auto [sane_key, sane_value] = sanitize_args(key, value, true);

        url.append(sane_key).append("=").append(sane_value).append("&");
    }

    // Remove the last char (which will be a & or ?)
    url.pop_back();

    auto [success, output] = utils::curl::get_direct(url);

    if (!success) {
        callback.code = 504;
        callback.end();

        LOG_F(WARNING, "Beatconnect search returned invalid response, message: %s", output.c_str());
        return;
    }

    json json_result;

    try {
        json_result = json::parse(output);
    }
    catch (const json::parse_error &ex) {
        LOG_F(ERROR, "Unable to parse json response from Beatconnect: %s.", ex.what());
        logging::sentry::exception(ex, __FILE__, __LINE__);

        callback.code = 504;
        callback.end(ex.what());
        return;
    }

    std::stringstream out;
    out << json_result["max_page"].get<int32_t>() << std::endl;

    for (auto &map_json : json_result["beatmaps"]) {
        std::string beatmap_id = std::to_string(map_json["id"].get<int32_t>());
        std::string last_updated = "-";

        if (map_json["last_updated"].is_string()) {
            last_updated = map_json["last_updated"];
        }

        out << beatmap_id << ".osz" << "|"; // Filename
        out << (std::string) map_json["artist"] << "|"; // Artist
        out << (std::string) map_json["title"] << "|"; // Song
        out << (std::string) map_json["creator"] << "|"; // Mapper
        out << 1 << "|"; // ?
        out << 0.0 << "|"; // Average Rating
        out << last_updated << "|"; // Last updated
        out << beatmap_id << "|"; // Beatmap id
        out << beatmap_id << "|"; // Beatmap id?
        out << 0 << "|"; // ?
        out << 0 << "|"; // ?
        out << 0 << "|"; // ?
        out << "|"; // Start of difficulties

        std::stringstream difficulties;
        difficulties << std::setprecision(2);

        for (auto &diff_json : map_json["beatmaps"]) {
            difficulties << (std::string) diff_json["version"] << " (";
            difficulties << (float) diff_json["difficulty"] << "★~";
            difficulties << "AR" << (float) diff_json["ar"] << "~";
            difficulties << "CS" << (float) diff_json["cs"] << "~";
            difficulties << "HP" << (float) diff_json["drain"] << "~";

            int32_t total_length = diff_json["total_length"];
            int32_t minutes = total_length / 60;
            int32_t seconds = total_length % 60;

            if (minutes > 0) {
                difficulties << minutes << "m";
            }

            difficulties << seconds << "s)" << "@";
            difficulties << diff_json["mode_int"].get<int32_t>() << ",";
        }

        std::string all_difficulties = difficulties.str();
        all_difficulties.pop_back();

        out << all_difficulties;
        out << "|"; // End of difficulties
        out << "\n"; // std::endl flushes additionally which is not something we want
    }

    callback.end(out.str());
}

void shiro::direct::beatconnect::search_np(crow::response& callback, std::unordered_map<std::string, std::string> parameters) {
    auto b = parameters.find("b");
    if (b == parameters.end()) {
        callback.code = 504;
        callback.end();

        return;
    }
    
    // Remove username from the request so the requesting user stays anonymous
    if (parameters.find("u") != parameters.end()) {
        parameters.erase("u");
    }

    // Remove password hash from the request so no credentials are leaked
    if (parameters.find("h") != parameters.end()) {
        parameters.erase("h");
    }

    int32_t beatmapset_id = 0;

    {
        sqlpp::mysql::connection db(db_connection->get_config());
        const tables::beatmaps beatmaps_tables{};

        int32_t beatmap_id = utils::strings::evaluate(b->second);
        auto result = db(sqlpp::select(beatmaps_tables.beatmapset_id).from(beatmaps_tables).where(beatmaps_tables.beatmap_id == beatmap_id).limit(1u));

        if (result.empty()) {
            callback.code = 504;
            callback.end();

            return;
        }

        auto& row = result.front();
        beatmapset_id = row.beatmapset_id;
    }

    std::string url = "https://beatconnect.io/api/beatmap/" + std::to_string(beatmapset_id) + "/";
    auto [success, output] = utils::curl::get_direct(url);

    if (!success) {
        callback.code = 504;
        callback.end();

        LOG_F(WARNING, "Beatconnect search returned invalid response, message: %s", output.c_str());
        return;
    }

    json json_result;

    try {
        json_result = json::parse(output);
    }
    catch (const json::parse_error& ex) {
        LOG_F(ERROR, "Unable to parse json response from Beatconnect: %s.", ex.what());
        logging::sentry::exception(ex, __FILE__, __LINE__);

        callback.code = 504;
        callback.end();

        return;
    }

    if (json_result["error"].is_object()) {
        callback.code = 504;
        callback.end();

        std::string error = json_result["error"].get<std::string>();
        LOG_F(ERROR, "Beatconnect JSON response contains errors: %s", error.c_str());

        return;
    }

    std::stringstream out;

    std::string beatmap_id = std::to_string(json_result["id"].get<int32_t>());
    bool has_video = json_result["video"].get<bool>();
    std::string last_updated = "-";
    
    if (json_result["last_updated"].is_string()) {
        last_updated = json_result["last_updated"];
    }

    out << beatmap_id << ".osz" << "|"; // Filename
    out << (std::string)json_result["artist"] << "|"; // Artist
    out << (std::string)json_result["title"] << "|"; // Song
    out << (std::string)json_result["creator"] << "|"; // Mapper

    // TODO: I really hope Beatconnect will fix this bug cuz this looks really shitty
    out << shiro::beatmaps::helper::fix_beatmap_status(
        json_result["ranked"].is_string()
        ? utils::strings::evaluate(json_result["ranked"].get<std::string>())
        : json_result["ranked"].get<int32_t>()
    ) << "|"; // Ranked status

    out << 0.0 << "|"; // Average Rating
    out << last_updated << "|"; // Last updated
    out << beatmap_id << "|"; // Beatmap id
    out << (int32_t)has_video << "|"; // Video?
    out << 0 << "|"; // ?
    out << 0 << "|"; // ?
    out << (has_video ? 7331 : 0); // Video size

    out << "\n"; // std::endl flushes additionally which is not something we want

    callback.end(out.str());
}

void shiro::direct::beatconnect::download(crow::response& callback, int32_t beatmap_id, bool no_video) {
    std::string id = std::to_string(beatmap_id);
    auto [success, output] = utils::curl::get_direct("https://beatconnect.io/api/beatmap/" + id + "/");

    if (!success) {
        callback.code = 504;
        callback.end();

        LOG_F(WARNING, "Beatconnect search returned invalid response, message: %s", output.c_str());
        return;
    }

    json json_result;

    try {
        json_result = json::parse(output);
    }
    catch (const json::parse_error &ex) {
        LOG_F(ERROR, "Unable to parse json response from Beatconnect: %s.", ex.what());
        logging::sentry::exception(ex, __FILE__, __LINE__);

        callback.code = 504;
        callback.end();

        return;
    }

    std::string unique_id = json_result["unique_id"];
    std::string url = "https://beatconnect.io/b/" + id + "/" + unique_id + "/";

    if (no_video) {
        url += "?novideo=1";
    }

    shiro::thread::curl_operations.push_and_forgot([&callback, url]() -> void {
        auto [success, output] = utils::curl::get_direct(url);

        if (!success) {
            callback.code = 504;
            callback.end();

            LOG_F(WARNING, "Beatconnect download returned invalid response, message: %s", output.c_str());
            return;
        }

        callback.set_header("Content-Type", "application/octet-stream; charset=UTF-8");
        callback.end(output);
    });
}

const std::string shiro::direct::beatconnect::name() const {
    return "Beatconnect";
}

std::tuple<std::string, std::string> shiro::direct::beatconnect::sanitize_args(std::string key, std::string value, bool url_escape) {
    if (key == "m") {
        sanitize_mode(value);
    }

    if (key == "r") {
        key = "s";
        sanitize_status(value);
    }

    if (key == "q") {
        sanitize_query(value);
    }

    // Escape parameters to be safely used in urls
    if (url_escape) {
        key = utils::curl::escape_url(key);
        value = utils::curl::escape_url(value);
    }

    return { key, value };
}

void shiro::direct::beatconnect::sanitize_mode(std::string &value) {
    int32_t mode = 0;

    if (!utils::strings::evaluate(value, mode)) {
        LOG_F(WARNING, "Unable to cast `%s` to int32_t.", value.c_str());

        return;
    }

    // Check if we're within enum range to avoid undefined behaviour
    if (mode < 0 || mode > 3) {
        value = "all";
        return;
    }

    value = utils::play_mode_to_string(static_cast<utils::play_mode>(mode), false);
}

void shiro::direct::beatconnect::sanitize_status(std::string &value) {
    static std::unordered_map<std::string, std::string> ranked_status_mapping = {
        { "0", "ranked" },
        { "1", "ranked" },
        { "2", "loved" },
        { "3", "qualified" },
        { "4", "unranked" },
        { "5", "unranked" }
    };

    if (ranked_status_mapping.find(value) != ranked_status_mapping.end()) {
        value = ranked_status_mapping.at(value);
        return;
    }

    value = "all";
}

void shiro::direct::beatconnect::sanitize_query(std::string &value) {
    std::transform(value.begin(), value.end(), value.begin(), ::tolower);

    boost::replace_all(value, "newest", "");
    boost::replace_all(value, "most played", "");
    boost::replace_all(value, "top rated", "");
}
