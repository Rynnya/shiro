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
#include "ipc_file.hh"

static std::shared_ptr<cpptoml::table> config_file = nullptr;

std::string shiro::config::ipc::backend_url = "https://c.yukime.ml/";
std::string shiro::config::ipc::frontend_url = "https://yukime.ml/";
std::string shiro::config::ipc::avatar_url = "https://a.yukime.ml/";
std::string shiro::config::ipc::beatmap_url = "https://osu.ppy.sh/b/";

void shiro::config::ipc::parse() {
    if (config_file != nullptr) {
        LOG_F(INFO, "Re-parsing ipc.toml file...");
    }

    try {
        config_file = cpptoml::parse_file("ipc.toml");
    }
    catch (const cpptoml::parse_exception &ex) {
        CAPTURE_EXCEPTION(ex);
        ABORT_F("Failed to parse ipc.toml file: {}.", ex.what());
    }

    backend_url = config_file->get_qualified_as<std::string>("meta.backend_url").value_or("https://c.yukime.ml/");
    frontend_url = config_file->get_qualified_as<std::string>("meta.frontend_url").value_or("https://yukime.ml/");
    avatar_url = config_file->get_qualified_as<std::string>("meta.avatar_url").value_or("https://a.yukime.ml/");
    beatmap_url = config_file->get_qualified_as<std::string>("meta.beatmap_url").value_or("https://osu.ppy.sh/b/");

    LOG_F(INFO, "Successfully parsed ipc.toml.");
}
