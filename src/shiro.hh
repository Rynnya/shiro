/*
 * shiro - High performance, high quality osu!Bancho C++ re-implementation
 * Copyright (C) 2018-2020 Marc3842h, czapek
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

#ifndef SHIRO_SHIRO_HH
#define SHIRO_SHIRO_HH

#include <chrono>
#include <ctime>
#include <memory>
#include <string>

#include "database/database.hh"
#include "redis/redis.hh"
#include "thirdparty/taskscheduler.hh"

namespace shiro {

    extern std::shared_ptr<database> db_connection;
    extern std::shared_ptr<redis> redis_connection;

    using namespace std::chrono_literals;
    extern tsc::TaskScheduler scheduler;

    extern std::time_t start_time;
    extern std::string commit;

    int init(int argc, char **argv);

    void destroy();

}

#endif //SHIRO_SHIRO_HH
