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
#include "bancho_file.hh"

static std::shared_ptr<cpptoml::table> config_file = nullptr;

std::string shiro::config::bancho::host = "127.0.0.1";
uint16_t shiro::config::bancho::port = 8080;
uint16_t shiro::config::bancho::concurrency = 64;

bool shiro::config::bancho::default_supporter = false;

std::string shiro::config::bancho::api_key = "osu! API key";

std::string shiro::config::bancho::alert = "Welcome to Shiro";
std::string shiro::config::bancho::title_image = "https://i.ppy.sh/motd.png";
std::string shiro::config::bancho::title_url = "https://osu.yukime.ml";

bool shiro::config::bancho::sentry_integration = true;
std::string shiro::config::bancho::sentry_dsn = "";
bool shiro::config::bancho::enable_breadcrumb = true;
int32_t shiro::config::bancho::breadcrumb_limit = 100;

void shiro::config::bancho::parse() {
    if (config_file != nullptr) {
        LOG_F(INFO, "Re-parsing bancho.toml file...");
    }

    try {
        config_file = cpptoml::parse_file("bancho.toml");
    }
    catch (const cpptoml::parse_exception &ex) {
        // We cannot capture exception here, because Sentry wasn't initialized and client is nullptr
        // CAPTURE_EXCEPTION(ex);
        ABORT_F("Failed to parse bancho.toml file: {}.", ex.what());
    }

    host = config_file->get_qualified_as<std::string>("server.host").value_or("127.0.0.1");
    port = config_file->get_qualified_as<uint16_t>("server.port").value_or(8080);
    concurrency = config_file->get_qualified_as<uint16_t>("server.concurrency").value_or(64);

    default_supporter = config_file->get_qualified_as<bool>("permissions.default_supporter").value_or(false);

    api_key = config_file->get_qualified_as<std::string>("api.key").value_or("osu! API key");

    alert = config_file->get_qualified_as<std::string>("motd.alert").value_or("Welcome to Shiro");
    title_image = config_file->get_qualified_as<std::string>("motd.title_image").value_or("https://i.ppy.sh/motd.png");
    title_url = config_file->get_qualified_as<std::string>("motd.title_url").value_or("https://osu.yukime.ml");

    sentry_integration = config_file->get_qualified_as<bool>("integrations.sentry").value_or(true);
    sentry_dsn = config_file->get_qualified_as<std::string>("integrations.sentry_dsn").value_or("");
    enable_breadcrumb = config_file->get_qualified_as<bool>("integrations.breadcrumb").value_or(true);
    breadcrumb_limit = config_file->get_qualified_as<int32_t>("integrations.breadcrumb_limit").value_or(100);

    LOG_F(INFO, "Successfully parsed bancho.toml.");
}
