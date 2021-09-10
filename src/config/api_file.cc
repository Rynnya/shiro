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

#include "../logger/sentry_logger.hh"
#include "../thirdparty/cpptoml.hh"
#include "../thirdparty/loguru.hh"
#include "api_file.hh"

static std::shared_ptr<cpptoml::table> config_file = nullptr;

bool shiro::config::api::deploy_enabled = false;
std::string shiro::config::api::deploy_key = "";
std::string shiro::config::api::deploy_command = "";

void shiro::config::api::parse() {
    if (config_file != nullptr)
        LOG_F(INFO, "Re-parsing api.toml file...");

    try {
        config_file = cpptoml::parse_file("api.toml");
    } catch (const cpptoml::parse_exception &ex) {
        logging::sentry::exception(ex);
        ABORT_F("Failed to parse api.toml file: %s.", ex.what());
    }

    deploy_enabled = config_file->get_qualified_as<bool>("deploy.enabled").value_or(false);
    deploy_key = config_file->get_qualified_as<std::string>("deploy.token").value_or("");
    deploy_command = config_file->get_qualified_as<std::string>("deploy.command").value_or("");

    // Passing arrays and booleans is not fully supported in CLI.
    // Thus, these options can't be configured like that.
    // Needs fixing in the future.

    if (!deploy_key.empty() || !deploy_enabled)
        return;

    deploy_enabled = false;
    LOG_F(WARNING, "Deployment has been activated but no deploy keys have been specified. Disabling deployment.");
}
