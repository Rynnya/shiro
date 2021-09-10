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

#ifndef SHIRO_DB_FILE_HH
#define SHIRO_DB_FILE_HH

#include <string>

namespace shiro::config::database {

    extern std::string address;
    extern uint32_t port;
    extern std::string database;
    extern std::string username;
    extern std::string password;

    extern std::string redis_address;
    extern uint32_t redis_port;
    extern std::string redis_password;

    void parse();

}

#endif //SHIRO_DB_FILE_HH
