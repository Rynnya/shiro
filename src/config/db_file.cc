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

#include "../logger/sentry_logger.hh"
#include "../thirdparty/cpptoml.hh"
#include "db_file.hh"

static std::shared_ptr<cpptoml::table> config_file = nullptr;

std::string shiro::config::database::address = "127.0.0.1";
uint32_t shiro::config::database::port = 3306;
std::string shiro::config::database::database = "shiro";
std::string shiro::config::database::username = "root";
std::string shiro::config::database::password = "hunter2";

std::string shiro::config::database::redis_address = "127.0.0.1";
uint32_t shiro::config::database::redis_port = 6379;
std::string shiro::config::database::redis_password = "";

void shiro::config::database::parse() {
    if (config_file != nullptr) {
        LOG_F(INFO, "Re-parsing database.toml file...");
    }

    try {
        config_file = cpptoml::parse_file("database.toml");
    }
    catch (const cpptoml::parse_exception &ex) {
        CAPTURE_EXCEPTION(ex);
        ABORT_F("Failed to parse database.toml file: {}.", ex.what());
    }

    address = config_file->get_qualified_as<std::string>("database.address").value_or("127.0.0.1");
    port = config_file->get_qualified_as<uint32_t>("database.port").value_or(3306);
    database = config_file->get_qualified_as<std::string>("database.database").value_or("shiro");
    username = config_file->get_qualified_as<std::string>("database.username").value_or("root");
    password = config_file->get_qualified_as<std::string>("database.password").value_or("hunter2");

    redis_address = config_file->get_qualified_as<std::string>("redis.address").value_or("127.0.0.1");
    redis_port = config_file->get_qualified_as<uint32_t>("redis.port").value_or(6379);
    redis_password = config_file->get_qualified_as<std::string>("redis.password").value_or("");

    LOG_F(INFO, "Successfully parsed database.toml.");
}
