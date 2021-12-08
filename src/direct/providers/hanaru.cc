/*
 * shiro - High performance, high quality osu!Bancho C++ re-implementation
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

#include <boost/interprocess/ipc/message_queue.hpp>

#include "../../beatmaps/beatmap_helper.hh"
#include "../../config/direct_file.hh"
#include "../../database/tables/beatmap_table.hh"
#include "../../logger/sentry_logger.hh"
#include "../../shiro.hh"
#include "../../utils/curler.hh"
#include "../../utils/string_utils.hh"
#include "hanaru.hh"

std::unordered_map<int32_t, std::weak_ptr<shiro::direct::beatmap_object>> shiro::direct::cache = {};
auto deleter = [&](shiro::direct::beatmap_object* obj) {
    shiro::direct::cache.erase(std::get<int32_t>(obj->get()));
    delete obj;
};

shiro::direct::hanaru::hanaru() {
    socket.set_message_handler([&](websocketpp::connection_hdl handle, client::message_ptr msg) {
        if (msg->get_opcode() != websocketpp::frame::opcode::binary) {
            // hanaru doesn't send anything other than ping, pong and binary
            return;
        }

        nlohmann::json payload;
        try {
            payload = nlohmann::json::parse(msg->get_payload());
        }
        catch (const json::parse_error& ex) {
            shiro::logging::sentry::exception(ex, __FILE__, __LINE__);
            LOG_F(ERROR, "Exception happend when handling websocket payload: %s", ex.what());

            return;
        }

        int32_t id = payload["id"].get<int32_t>();

        std::promise<std::tuple<bool, int32_t, std::string>> promise;
        cache[id] = std::shared_ptr<beatmap_object>(new beatmap_object(promise.get_future()), deleter);

        promise.set_value({ payload["status"].get<int32_t>() == 200, id, payload["data"].get<std::string>() });
    });

    socket.set_max_message_size(256 * 1000 * 1000);

    websocketpp::lib::error_code ec;
    connection_ptr = socket.get_connection("ws://127.0.0.1:" + std::to_string(config::direct::port) + "/", ec);
    if (ec) {
        connection_ptr.reset();
        connection_ptr = nullptr;
        return;
    }

    socket.connect(connection_ptr);
}

std::tuple<bool, std::string> shiro::direct::hanaru::search(std::unordered_map<std::string, std::string> parameters) {
    sqlpp::mysql::connection db(db_connection->get_config());
    const tables::beatmaps beatmaps_tables {};

    auto statement = sqlpp::dynamic_select(db, all_of(beatmaps_tables)).from(beatmaps_tables).dynamic_where().order_by(beatmaps_tables.beatmapset_id.desc()).limit(1000U);

    for (const auto &[header, value] : parameters) {
        if (header == "r") {
            int32_t status = sanitize_status(value);
            if (status != -3) {
                statement.where.add(beatmaps_tables.ranked_status == status);
            }
            continue;
        }

        if (header == "m") {
            int32_t mode = sanitize_mode(value);
            if (mode != -1) {
                statement.where.add(beatmaps_tables.mode == mode);
            }
            continue;
        }
    }

    auto result = db(statement);

    std::stringstream out;
    out << 1 << std::endl;

    std::stringstream difficulties;
    difficulties << std::setprecision(2);

    int32_t previous_id = result.begin()->beatmapset_id;
    bool requires_end = false;
    for (const auto& map : result) {
        if (previous_id == map.beatmapset_id) {
            difficulties << map.difficulty_name.value() << " (";
            switch (map.mode) {
            case 0:
                default: {
                    difficulties << map.difficulty_std.value();
                    break;
                }
                case 1: {
                    difficulties << map.difficulty_taiko.value();
                    break;
                }
                case 2: {
                    difficulties << map.difficulty_ctb.value();
                    break;
                }
                case 3: {
                    difficulties << map.difficulty_mania.value();
                    break;
                }
            }

            difficulties << "★~" << map.bpm.value() << "♫~";
            difficulties << "AR" << map.ar.value() << "~";
            difficulties << "OD" << map.od.value() << "~";
            difficulties << "CS" << map.cs.value() << "~";
            difficulties << "HP" << map.hp.value() << "~";

            int32_t total_length = map.hit_length;
            int32_t minutes = total_length / 60;
            int32_t seconds = total_length % 60;

            difficulties << minutes << "m";
            difficulties << seconds << "s)" << "@";
            difficulties << map.mode.value() << ",";
        }
        else {
            if (requires_end) {
                std::string diffs = difficulties.str();
                diffs.pop_back();
                out << diffs;
                out << "|\n";
            }

            requires_end = true;
            previous_id = map.beatmapset_id;

            std::string beatmap_id = std::to_string(map.beatmap_id);
            int64_t latest_update = map.latest_update;
            std::tm time = *std::gmtime(&latest_update);

            out << beatmap_id << ".osz" << "|"; // Filename
            out << map.artist.value() << "|"; // Artist
            out << map.title.value() << "|"; // Song
            out << map.creator.value() << "|"; // Mapper
            out << 1 << "|"; // ?
            out << 0.0 << "|"; // Average Rating
            out << std::put_time(&time, "%Y-%m-%dT%H:%M:%SZ") << "|"; // Last updated
            out << beatmap_id << "|"; // Beatmap id
            out << beatmap_id << "|"; // Beatmap id?
            out << 0 << "|"; // ?
            out << 0 << "|"; // ?
            out << 0 << "|"; // ?
            out << "|"; // Start of difficulties

            difficulties.clear();

            difficulties << map.difficulty_name.value() << " (";
            switch (map.mode) {
                case 0:
                default: {
                    difficulties << map.difficulty_std.value();
                    break;
                }
                case 1: {
                    difficulties << map.difficulty_taiko.value();
                    break;
                }
                case 2: {
                    difficulties << map.difficulty_ctb.value();
                    break;
                }
                case 3: {
                    difficulties << map.difficulty_mania.value();
                    break;
                }
            }

            difficulties << "★~" << map.bpm.value() << "♫~";
            difficulties << "AR" << map.ar.value() << "~";
            difficulties << "OD" << map.od.value() << "~";
            difficulties << "CS" << map.cs.value() << "~";
            difficulties << "HP" << map.hp.value() << "~";

            int32_t total_length = map.hit_length;
            int32_t minutes = total_length / 60;
            int32_t seconds = total_length % 60;

            difficulties << minutes << "m";
            difficulties << seconds << "s)" << "@";
            difficulties << map.mode.value() << ",";
        }
    }

    return { true, out.str() };
}

std::tuple<bool, std::string> shiro::direct::hanaru::search_np(std::unordered_map<std::string, std::string> parameters) {
    auto b = parameters.find("b");
    if (b == parameters.end()) {
        return { false, "'beatmap_id' not provided" };
    }

    sqlpp::mysql::connection db(db_connection->get_config());
    const tables::beatmaps beatmaps_tables{};

    int32_t _beatmap_id = utils::strings::safe_ll(b->second);
    auto result = db(sqlpp::select(beatmaps_tables.beatmapset_id).from(beatmaps_tables).where(beatmaps_tables.beatmap_id == _beatmap_id));

    if (result.empty()) {
        return { false, "Beatmap not loaded to database" };
    }

    auto& _result = result.front();
    int32_t beatmapset_id = _result.beatmapset_id;

    auto [success, output] = shiro::utils::curl::get_direct(config::direct::hanaru_url + "/s/" + std::to_string(beatmapset_id));

    if (!success) {
        return { false, output };
    }

    nlohmann::json json_result;
    try {
        json_result = nlohmann::json::parse(output);
    }
    catch (const json::parse_error& ex) {
        LOG_F(ERROR, "Unable to parse json response from Hanaru: %s.", ex.what());
        logging::sentry::exception(ex, __FILE__, __LINE__);

        return { false, ex.what() };
    }

    std::stringstream out;

    json_result = json_result[0];
    std::string beatmap_id = std::to_string(json_result["beatmapset_id"].get<int32_t>());
    bool has_video = false; // Hanaru doesn't provide beatmaps with video
    std::string last_updated = "-";

    if (json_result["latest_update"].is_string()) {
        last_updated = json_result["latest_update"];
    }

    out << beatmap_id << ".osz" << "|"; // Filename
    out << json_result["artist"].get<std::string>() << "|"; // Artist
    out << json_result["title"].get<std::string>() << "|"; // Song
    out << json_result["creator"].get<std::string>() << "|"; // Mapper
    out << shiro::beatmaps::helper::fix_beatmap_status(json_result["ranked_status"].get<int32_t>()) << "|"; // Ranked status
    out << 0.0 << "|"; // Average Rating
    out << last_updated << "|"; // Last updated
    out << beatmap_id << "|"; // Beatmap id
    out << beatmap_id << "|"; // Beatmap id?
    out << 0 << "|"; // Video?
    out << 0 << "|"; // ?
    out << 0 << "|"; // ?
    out << 0; // Video size

    out << "\n"; // std::endl flushes additionally which is not something we want

    return { true, out.str() };
}

std::tuple<bool, std::string> shiro::direct::hanaru::download(int32_t beatmap_id, bool no_video) {
    // hanaru doesn't provide beatmapsets with video
    static_cast<void>(no_video);

    if (connection_ptr == nullptr) {
        return { false, "Failed to connect through websocket" };
    }
    
    std::shared_ptr<beatmap_object> object = std::make_shared<beatmap_object>();
    auto it = cache.find(beatmap_id);

    if (it == cache.end()) {
        cache.emplace(beatmap_id, object);
        connection_ptr->send(std::to_string(beatmap_id));
    }
    else {
        object = it->second.lock();
    }

    const auto [success, id, data] = object->get();
    return { success, data };
}

const std::string shiro::direct::hanaru::name() const {
    return "Hanaru";
}

int32_t shiro::direct::hanaru::sanitize_mode(const std::string& value) {
    int32_t mode = 0;

    if (!utils::strings::safe_int(value, mode)) {
        return -1;
    }

    // Check if we're within enum range to avoid undefined behaviour
    if (mode < 0 || mode > 3) {
        return -1;
    }

    return mode;
}

int32_t shiro::direct::hanaru::sanitize_status(const std::string& value) {
    int32_t status = 0;

    if (utils::strings::safe_int(value, status)) {
        return beatmaps::helper::fix_beatmap_status(status);
    }

    return -3;
}