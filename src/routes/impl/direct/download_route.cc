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

#include "../../../direct/direct_provider.hh"
#include "../../../utils/string_utils.hh"
#include "../../../thirdparty/loguru.hh"
#include "download_route.hh"

void shiro::routes::direct::download::handle(const crow::request &request, crow::response &response, std::string args) {
    response.set_header("Content-Type", "text/plain; charset=UTF-8");
    response.set_header("cho-server", "shiro (https://github.com/Rynnya/shiro)");

    const char &last_char = args.back();
    bool no_video = last_char == 'n';
    if (no_video) {
        args.pop_back();
    }

    int32_t id = 0;

    if (!utils::strings::evaluate(args, id)) {
        response.code = 400;
        response.end("Invalid beatmap id");
        return;
    }

    // Provider has failed sanity check, thus we can't provide Direct
    if (shiro::direct::provider == nullptr) {
        response.code = 502;
        response.end();
        return;
    }

    shiro::direct::provider->download(response, id, no_video);
}
