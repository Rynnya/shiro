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

#ifndef SHIRO_SENTRY_LOGGER_HH
#define SHIRO_SENTRY_LOGGER_HH

#include <string>

#include "../thirdparty/crow.hh"
#include "../thirdparty/sentry/crow.hh"
#include "../thirdparty/naga.hh"
#include "../thirdparty/taskscheduler.hh"

namespace shiro::logging::sentry {

    extern std::shared_ptr<nlohmann::crow> client;
    extern tsc::TaskScheduler scheduler;

    void init();

    void callback(std::any& user_data, const naga::log_message& message);

    void fatal_callback(const naga::log_message& message);

    void exception(const std::exception& ex, const char* file, const unsigned int line);

    void exception(const std::exception_ptr& ptr, const char* file, const unsigned int line);

    void http_request_out(const std::string &url, const std::string &method = "GET", int32_t status_code = 200, const std::string &reason = "OK");

    void http_request_in(const ::crow::request &request);

    std::string verbosity_to_sentry_level(const naga::log_level& verbosity);

    // Macro to discard __FILE__ and __LINE__ macroes
    #define CAPTURE_EXCEPTION(EX) shiro::logging::sentry::exception(EX, __FILE__, __LINE__)

}

#endif //SHIRO_SENTRY_LOGGER_HH
