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

#include <boost/algorithm/string.hpp>
#include <ctime>
#include <fstream>

#include "../thirdparty/fmt/format.hh"
#include "../utils/filesystem.hh"
#include "../shiro.hh"
#include "index_view.hh"

std::string shiro::views::index::replace_time(const std::string &view) {
    std::time_t seconds = static_cast<std::time_t>(std::difftime(std::time(nullptr), start_time));
    std::tm *p = std::gmtime(&seconds);

    int32_t days = p->tm_yday;
    int32_t hours = p->tm_hour;
    int32_t minutes = p->tm_min;
    int32_t secs = p->tm_sec;

    return boost::replace_all_copy(view, "{{uptime}}", 
        fmt::format(
            "{} {} {} {} {} {} {} {}",
            days, days == 1 ? "day" : "days",
            hours, hours == 1 ? "hour" : "hours",
            minutes, minutes == 1 ? "minute" : "minutes",
            secs, secs == 1 ? "second" : "seconds"
        )
    );
}

std::string shiro::views::index::get_view() {
    static std::string view;
    static std::once_flag once;

    std::call_once(once, [&]() {

        {
            std::ifstream stream("shiro.html");

            std::stringstream buffer;
            buffer << stream.rdbuf();

            view = buffer.str();
        }

        if (fs::exists("commit.txt")) {
            std::ifstream stream("commit.txt");

            std::stringstream buffer;
            buffer << stream.rdbuf();

            commit = buffer.str();
        }

    });

    // Templates are overrated anyway
    boost::replace_all(view, "{{hash}}", commit);

    return replace_time(view);
}
