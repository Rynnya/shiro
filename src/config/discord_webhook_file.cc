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

#include "../logger/sentry_logger.hh"
#include "../thirdparty/cpptoml.hh"
#include "../thirdparty/loguru.hh"
#include "discord_webhook_file.hh"

static std::shared_ptr<cpptoml::table> config_file = nullptr;

bool shiro::config::discord_webhook::enabled = false;
std::string shiro::config::discord_webhook::url = "";

bool shiro::config::discord_webhook::override_user = true;
std::string shiro::config::discord_webhook::name = "Shiro";

void shiro::config::discord_webhook::parse() {
    if (config_file != nullptr) {
        LOG_F(INFO, "Re-parsing discord_webhook.toml file...");
    }

    try {
        config_file = cpptoml::parse_file("discord_webhook.toml");
    }
    catch (const cpptoml::parse_exception& ex) {
        logging::sentry::exception(ex, __FILE__, __LINE__);
        ABORT_F("Failed to parse discord_webhook.toml file: %s.", ex.what());
    }

    enabled = config_file->get_qualified_as<bool>("webhook.enabled").value_or(false);
    url = config_file->get_qualified_as<std::string>("webhook.url").value_or("");

    if (!enabled || url == "") {
        LOG_F(INFO, "Successfully parsed discord_webhook.toml.");
        return;
    }

    override_user = config_file->get_qualified_as<bool>("webhook_user.override").value_or(false);

    if (!override_user) {
        LOG_F(INFO, "Successfully parsed discord_webhook.toml.");
        return;
    }

    name = config_file->get_qualified_as<std::string>("webhook_user.username").value_or("Shiro");

    LOG_F(INFO, "Successfully parsed discord_webhook.toml.");
}
