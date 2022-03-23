/*
 * shiro - High performance, high quality osu!Bancho C++ re-implementation
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

#include "../../../direct/direct_provider.hh"
#include "search_set_route.hh"

void shiro::routes::direct::search::set::handle(const crow::request& request, crow::response& response) {
    response.set_header("Content-Type", "text/plain; charset=UTF-8");
    response.set_header("cho-server", "shiro (https://github.com/Rynnya/shiro)");

    // Provider has failed sanity check, thus we can't provide Direct
    if (shiro::direct::provider == nullptr) {
        response.code = 502;
        response.end();
        return;
    }

    shiro::direct::provider->search_np(response, request.url_params.get_all());
}
