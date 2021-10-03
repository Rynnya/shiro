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

#include "../../../utils/curler.hh"
#include "../../../thirdparty/loguru.hh"
#include "maps_route.hh"

void shiro::routes::direct::maps::handle(const crow::request& request, crow::response& response, std::string args)
{
	response.set_header("Content-Type", "text/plain; charset=UTF-8");
	response.set_header("cho-server", "shiro (https://github.com/Marc3842h/shiro)");

	auto [success, result] = shiro::utils::curl::get("https://osu.ppy.sh/web/maps/" + args);

	if (!success)
	{
		response.code = 504;
		response.end();

		LOG_F(WARNING, "Maps returned invalid response, message: %s", result.c_str());

		return;
	}

	response.set_header("Content-Type", "application/octet-stream; charset=UTF-8");
	response.end(result);
}