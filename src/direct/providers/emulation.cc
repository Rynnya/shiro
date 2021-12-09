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

#include "../../config/direct_file.hh"
#include "../../thread/thread_pool.hh"
#include "../../utils/curler.hh"
#include "emulation.hh"

void shiro::direct::emulation::search(crow::response&& callback, std::unordered_map<std::string, std::string> parameters) {
    // Remove username from the request so the requesting user stays anonymous
    if (parameters.find("u") != parameters.end()) {
        parameters.erase("u");
    }

    // Remove password hash from the request so no credentials are leaked
    if (parameters.find("h") != parameters.end()) {
        parameters.erase("h");
    }

    std::string url = config::direct::base_url + (parameters.find("b") == parameters.end() ? "/web/osu-search-set.php?" : "/web/osu-search.php?");

    for (const auto &[key, value] : parameters) {
        url.append(key).append("=").append(utils::curl::escape_url(value)).append("&");
    }

    // Remove the last char (which will be a & or ?)
    url.pop_back();

    uint32_t index = hold_callback(std::move(callback));

    shiro::thread::curl_operations.push_and_forgot([this, url, index]() -> void {
        auto [success, output] = utils::curl::get_direct(url);
        crow::response& callback = holder[index];

        if (!success) {
            callback.code = 504;
            callback.end();

            holder.erase(index);

            return;
        }

        callback.end(output);

        holder.erase(index);
    });
}

void shiro::direct::emulation::search_np(crow::response&& callback, std::unordered_map<std::string, std::string> parameters) {
    return this->search(std::move(callback), parameters);
}

void shiro::direct::emulation::download(crow::response&& callback, int32_t beatmap_id, bool no_video) {
    std::string url = config::direct::mirror_url + "/d/" + std::to_string(beatmap_id);

    if (no_video) {
        url.append("?novideo=yes");
    }

    uint32_t index = hold_callback(std::move(callback));

    shiro::thread::curl_operations.push_and_forgot([this, url, index]() -> void {
        auto [success, output] = utils::curl::get_direct(url);
        crow::response& callback = holder[index];

        if (!success) {
            callback.code = 504;
            callback.end();

            holder.erase(index);

            return;
        }

        callback.set_header("Content-Type", "application/octet-stream; charset=UTF-8");
        callback.end(output);

        holder.erase(index);
    });
}

const std::string shiro::direct::emulation::name() const {
    return "Emulation";
}
