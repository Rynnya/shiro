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

#include "../thirdparty/naga.hh"
#include "route_logger.hh"

void shiro::logging::route_logger::log(std::string message, crow::LogLevel level) {
    switch (level) {
        case crow::LogLevel::Info: {
            LOG_F(INFO, "{}", message);
            break;
        }
        case crow::LogLevel::Warning: {
            LOG_F(WARNING, "{}", message);
            break;
        }
        case crow::LogLevel::Error: {
            #undef ERROR // Windows bug
            LOG_F(ERROR, "{}", message);
            break;
        }
        case crow::LogLevel::Critical: {
            ABORT_F("{}", message);
            break;
        }
        default: {
            // Do nothing
            break;
        }
    }
}
