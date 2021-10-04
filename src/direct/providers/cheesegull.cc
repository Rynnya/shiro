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
#include "../../utils/string_utils.hh"
#include "../../config/direct_file.hh"
#include "../../logger/sentry_logger.hh"
#include "../../thirdparty/loguru.hh"
#include "../../utils/curler.hh"
#include "../../utils/play_mode.hh"
#include "cheesegull.hh"

std::tuple<bool, std::string> shiro::direct::cheesegull::search(std::unordered_map<std::string, std::string> parameters)
{
    // Remove username from the request so the requesting user stays anonymous
    if (parameters.find("u") != parameters.end())
        parameters.erase("u");

    // Remove password hash from the request so no credentials are leaked
    if (parameters.find("h") != parameters.end())
        parameters.erase("h");

    std::string url = config::direct::search_url + "/search?";

    for (const auto& [key, value] : parameters) {
        auto [sane_key, sane_value] = sanitize_args(key, value, true);

        url.append(sane_key).append("=").append(sane_value).append("&");
    }

    url.append("amount=").append(std::to_string(config::direct::beatmaps_amount));

    auto [success, output] = utils::curl::get_direct(url);

    if (!success)
        return { false, output };

    json json_result;

    try
    {
        json_result = json::parse(output);
    }
    catch (const json::parse_error& ex) 
    {
        LOG_F(ERROR, "Unable to parse json response from Cheesegull: %s.", ex.what());
        logging::sentry::exception(ex);

        return { false, ex.what() };
    }

    std::stringstream out;
    out << 1000 << std::endl;

    for (auto& map_json : json_result)
    {
        std::string beatmap_id = std::to_string(map_json["SetID"].get<int32_t>());
        std::string last_updated = "-";

        if (map_json["LastUpdate"].is_string())
            last_updated = map_json["LastUpdate"];

        out << beatmap_id << ".osz" << "|"; // Filename
        out << (std::string)map_json["Artist"] << "|"; // Artist
        out << (std::string)map_json["Title"] << "|"; // Song
        out << (std::string)map_json["Creator"] << "|"; // Mapper
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

        for (auto& diff_json : map_json["ChildrenBeatmaps"]) 
        {
            difficulties << (std::string)diff_json["DiffName"] << " (";
            difficulties << (float)diff_json["DifficultyRating"] << "★~";
            difficulties << "AR" << (float)diff_json["AR"] << "~";
            difficulties << "CS" << (float)diff_json["CS"] << "~";
            difficulties << "HP" << (float)diff_json["HP"] << "~";

            int32_t total_length = diff_json["TotalLength"];
            int32_t minutes = total_length / 60;
            int32_t seconds = total_length % 60;

            if (minutes > 0)
                difficulties << minutes << "m";

            difficulties << seconds << "s)" << "@";
            difficulties << diff_json["Mode"].get<int32_t>() << ",";
        }

        std::string all_difficulties = difficulties.str();
        all_difficulties.pop_back();

        out << all_difficulties;
        out << "|"; // End of difficulties
        out << "\n"; // std::endl flushes additionally which is not something we want
    }

    return { true, out.str() };
}

std::tuple<bool, std::string> shiro::direct::cheesegull::search_np(std::unordered_map<std::string, std::string> parameters)
{
    auto b = parameters.find("b");
    if (b == parameters.end())
        return { false, "b not provided" };

    // Remove username from the request so the requesting user stays anonymous
    if (parameters.find("u") != parameters.end())
        parameters.erase("u");

    // Remove password hash from the request so no credentials are leaked
    if (parameters.find("h") != parameters.end())
        parameters.erase("h");

    std::string set_url = config::direct::search_url + "/b/" + b->second;
    auto [set_success, set_output] = utils::curl::get_direct(set_url);

    if (!set_success)
        return { false, set_output };

    json set_result;

    try
    {
        set_result = json::parse(set_output);
    }
    catch (const json::parse_error& ex)
    {
        LOG_F(ERROR, "Unable to parse json response from Cheesegull: %s.", ex.what());
        logging::sentry::exception(ex);

        return { false, ex.what() };
    }

    std::string url = config::direct::search_url + "/s/" + std::to_string(set_result["ParentSetID"].get<int32_t>());
    auto [success, output] = utils::curl::get_direct(url);

    if (!success)
        return { false, output };

    json json_result;

    try
    {
        json_result = json::parse(output);
    }
    catch (const json::parse_error& ex)
    {
        LOG_F(ERROR, "Unable to parse json response from Cheesegull: %s.", ex.what());
        logging::sentry::exception(ex);

        return { false, ex.what() };
    }

    std::stringstream out;

    std::string beatmap_id = std::to_string(json_result["SetID"].get<int32_t>());
    bool has_video = json_result["HasVideo"].get<bool>();
    std::string last_updated = "-";

    if (json_result["LastUpdate"].is_string())
        last_updated = json_result["LastUpdate"];

    out << beatmap_id << ".osz" << "|"; // Filename
    out << (std::string)json_result["Artist"] << "|"; // Artist
    out << (std::string)json_result["Title"] << "|"; // Song
    out << (std::string)json_result["Creator"] << "|"; // Mapper
    out << shiro::beatmaps::helper::fix_beatmap_status(json_result["RankedStatus"].get<int32_t>()) << "|"; // Ranked status
    out << 0.0 << "|"; // Average Rating
    out << last_updated << "|"; // Last updated
    out << beatmap_id << "|"; // Beatmap id
    out << beatmap_id << "|"; // Beatmap id?
    out << (int32_t)has_video << "|"; // Video?
    out << 0 << "|"; // ?
    out << 0 << "|"; // ?
    out << (has_video ? 7331 : 0); // Video size (faked cuz nobody provide this)

    out << "\n"; // std::endl flushes additionally which is not something we want

    return { true, out.str() };
}

std::tuple<bool, std::string> shiro::direct::cheesegull::download(int32_t beatmap_id, bool no_video)
{
    std::string id = std::to_string(beatmap_id);
    std::string url = config::direct::download_url + "/d/" + id;

    if (no_video)
        url += "?n=1";

    return utils::curl::get_direct(url);
}

const std::string shiro::direct::cheesegull::name() const
{
	return "Cheesegull";
}

std::tuple<std::string, std::string> shiro::direct::cheesegull::sanitize_args(std::string key, std::string value, bool url_escape)
{
    if (key == "m")
    {
        key = "mode";
        sanitize_mode(value);
    }

    if (key == "r")
    {
        key = "status";
        sanitize_status(value);
    }

    if (key == "q")
    {
        key = "query";
        sanitize_query(value);
    }

    if (key == "p")
    {
        key = "offset";
        sanitize_offset(value);
    }

    // Escape parameters to be safely used in urls
    if (url_escape) 
    {
        key = utils::curl::escape_url(key);
        value = utils::curl::escape_url(value);
    }

    return { key, value };
}

void shiro::direct::cheesegull::sanitize_mode(std::string& value)
{
    int mode = utils::strings::safe_int(value);
    if (mode == -1 || mode < 0 || mode > 3)
        value = "-1";
}

void shiro::direct::cheesegull::sanitize_status(std::string& value)
{
    int status = utils::strings::safe_int(value);
    if (status == -1)
        return;

    switch (status)
    {
        case 8:
        {
            value = "4";
            return;
        }
        case 2:
        {
            value = "0";
            return;
        }
        case 5:
        {
            value = "-2";
            return;
        }
        default:
        {
            value = "1";
            return;
        }
    }
}

void shiro::direct::cheesegull::sanitize_query(std::string& value)
{
    std::transform(value.begin(), value.end(), value.begin(), ::tolower);

    boost::replace_all(value, "newest", "");
    boost::replace_all(value, "most played", "");
    boost::replace_all(value, "top rated", "");
}

void shiro::direct::cheesegull::sanitize_offset(std::string& value)
{
    int page = utils::strings::safe_int(value);
    if (page == -1)
        return;

    value = std::to_string(page * config::direct::beatmaps_amount);
}
