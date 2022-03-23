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

#include "../../config/direct_file.hh"
#include "../../logger/sentry_logger.hh"
#include "../../thread/thread_pool.hh"
#include "../../utils/curler.hh"
#include "../../utils/string_utils.hh"
#include "emulation.hh"

void shiro::direct::emulation::search(crow::response& callback, std::unordered_map<std::string, std::string> parameters) {
    // Remove username from the request so the requesting user stays anonymous
    if (parameters.find("u") != parameters.end()) {
        parameters.erase("u");
    }

    // Remove password hash from the request so no credentials are leaked
    if (parameters.find("h") != parameters.end()) {
        parameters.erase("h");
    }

    std::string url = fmt::format(
        "{}{}",
        config::direct::base_url,
        (parameters.find("b") == parameters.end() ? "/web/osu-search-set.php?" : "/web/osu-search.php?")
    );

    for (const auto &[key, value] : parameters) {
        url.append(key).append("=").append(utils::curl::escape_url(value)).append("&");
    }

    // Remove the last char (which will be a & or ?)
    url.pop_back();

    shiro::thread::curl_operations.push_and_forgot([&callback, url]() -> void {
        auto [success, output] = utils::curl::get_direct(url);

        if (!success) {
            callback.code = 504;
            callback.end();

            LOG_F(WARNING, "Emulation search returned invalid response, message: {}", output);
            return;
        }

        callback.end(output);
    });
}

void shiro::direct::emulation::search_np(crow::response& callback, std::unordered_map<std::string, std::string> parameters) {
    return this->search(callback, parameters);
}

void shiro::direct::emulation::download(crow::response& callback, int32_t beatmap_id, bool no_video) {
    std::string url = fmt::format("{}/d/{}", config::direct::mirror_url, beatmap_id);

    if (no_video) {
        url.append("?novideo=yes");
    }

    shiro::thread::curl_operations.push_and_forgot([&callback, url]() -> void {
        auto [success, output] = utils::curl::get_direct(url);

        if (!success) {
            callback.code = 504;
            callback.end();

            if (output.size() > 100) {
                LOG_F(WARNING, "Emulation search returned invalid response with huge message, size: {}", output.size());
                return;
            }

            LOG_F(WARNING, "Emulation search returned invalid response, message: {}", output);
            return;
        }

        callback.set_header("Content-Type", "application/octet-stream; charset=UTF-8");
        callback.end(output);
    });
}

const std::string shiro::direct::emulation::name() const {
    return "Emulation";
}
