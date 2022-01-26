﻿/*
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

shiro::direct::hanaru::hanaru() {
    socket.set_access_channels(websocketpp::log::alevel::none);
    socket.set_error_channels(websocketpp::log::elevel::none);

    socket.set_message_handler(websocketpp::lib::bind(&hanaru::on_message, this, websocketpp::lib::placeholders::_1, websocketpp::lib::placeholders::_2));

    socket.set_max_message_size(256 * 1000 * 1000);
    socket.init_asio();
    socket.set_reuse_addr(true);
    socket.set_listen_backlog(64);

    std::thread running_client([&]() {
        while (true) {
            try {
                socket.reset();

                websocketpp::lib::error_code ec;
                connection_ptr = socket.get_connection("ws://127.0.0.1:" + std::to_string(config::direct::port) + "/", ec);
                if (ec) {
                    std::string msg = ec.message();
                    LOG_F(ERROR, "Cannot connect through websocket: %s", msg.c_str());

                    connection_ptr = nullptr;
                    std::this_thread::sleep_for(std::chrono::seconds(5));
                    continue;
                }

                socket.connect(connection_ptr);
                socket.run();
            }
            catch (...) {
                // Set connection to null so we doesn't leak memory through cache
                connection_ptr = nullptr;
            }
        }
    });

    running_client.detach();
}

void shiro::direct::hanaru::search(crow::response& callback, std::unordered_map<std::string, std::string> parameters) {
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

            difficulties = std::stringstream();
            difficulties << std::setprecision(2);

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

    callback.end(out.str());
}

void shiro::direct::hanaru::search_np(crow::response& callback, std::unordered_map<std::string, std::string> parameters) {
    auto beatmap_id = parameters.find("b");
    if (beatmap_id == parameters.end()) {
        callback.code = 504;
        callback.end();

        return;
    }

    std::string url =
        (config::direct::hanaru_url.find("localhost") != std::string::npos
            ? "127.0.0.1:" + std::to_string(config::direct::port)
            : config::direct::hanaru_url
        ) + "/b/" + beatmap_id->second;
    auto [success, output] = shiro::utils::curl::get_direct(url);

    if (!success) {
        callback.code = 504;
        callback.end();

        LOG_F(WARNING, "Hanaru search_np returned invalid response, message: %s", output.c_str());
        return;
    }

    nlohmann::json json_result;
    try {
        json_result = nlohmann::json::parse(output);
    }
    catch (const json::parse_error& ex) {
        LOG_F(ERROR, "Unable to parse json response from Hanaru: %s.", ex.what());
        logging::sentry::exception(ex, __FILE__, __LINE__);

        callback.code = 504;
        callback.end();

        return;
    }

    std::stringstream out;

    std::string beatmapset_id = std::to_string(json_result["beatmapset_id"].get<int32_t>());
    bool has_video = false; // Hanaru doesn't provide beatmaps with video
    std::string last_updated = "-";

    if (json_result["latest_update"].is_string()) {
        last_updated = json_result["latest_update"];
    }

    out << beatmapset_id << ".osz" << "|"; // Filename
    out << json_result["artist"].get<std::string>() << "|"; // Artist
    out << json_result["title"].get<std::string>() << "|"; // Song
    out << json_result["creator"].get<std::string>() << "|"; // Mapper
    out << shiro::beatmaps::helper::fix_beatmap_status(json_result["ranked_status"].get<int32_t>()) << "|"; // Ranked status
    out << 0.0 << "|"; // Average Rating
    out << last_updated << "|"; // Last updated
    out << beatmapset_id << "|"; // Beatmap id
    out << beatmapset_id << "|"; // Beatmap id?
    out << 0 << "|"; // Video?
    out << 0 << "|"; // ?
    out << 0 << "|"; // ?
    out << 0; // Video size

    out << "\n"; // std::endl flushes additionally which is not something we want

    callback.end(out.str());
}

void shiro::direct::hanaru::download(crow::response& callback, int32_t beatmap_id, bool no_video) {
    // hanaru doesn't provide beatmapsets with video
    static_cast<void>(no_video);

    if (connection_ptr == nullptr) {
        callback.code = 502;
        callback.end();

        return;
    }

    bool required_request = false;
    {
        std::lock_guard<std::mutex> lock(mtx);

        required_request = cache.find(beatmap_id) == cache.end();
        cache[beatmap_id].push_back([&callback](int32_t code, const std::string& data) {

            if (code == 200) {
                callback.set_header("Content-Type", "application/octet-stream; charset=UTF-8");
                callback.end(data);
                return;
            }

            callback.code = 504;
            callback.end(data);
        });
    }

    if (required_request) {
        connection_ptr->send(std::to_string(beatmap_id));
    }
}

const std::string shiro::direct::hanaru::name() const {
    return "Hanaru";
}

void shiro::direct::hanaru::on_message(websocketpp::connection_hdl handle, client::message_ptr msg) {
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
    int32_t code = payload["status"].get<int32_t>();
    std::string data = websocketpp::base64_decode(payload["data"].get<std::string>());

    std::lock_guard<std::mutex> lock(mtx);

    for (auto& callback : cache[id]) {
        callback(code, data);
    }

    cache.erase(id);
}

int32_t shiro::direct::hanaru::sanitize_mode(const std::string& value) {
    int32_t mode = 0;

    if (!utils::strings::evaluate(value, mode)) {
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

    if (utils::strings::evaluate(value, status)) {
        return beatmaps::helper::fix_beatmap_status(status);
    }

    return -3;
}