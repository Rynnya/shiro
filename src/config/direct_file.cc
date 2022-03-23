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
#include "direct_file.hh"

static std::shared_ptr<cpptoml::table> config_file = nullptr;

bool shiro::config::direct::enabled = true;
int32_t shiro::config::direct::provider = 1;

// Provider 0: Hanaru
uint32_t shiro::config::direct::port = 8090;
std::string shiro::config::direct::hanaru_url = "https://mirror.yukime.ml";

// Provider 1: Emulation
std::string shiro::config::direct::base_url = "https://yukime.ml";
std::string shiro::config::direct::mirror_url = "https://mirror.yukime.ml";

// Provider 2: Beatconnect
std::string shiro::config::direct::api_key;

// Provider 3: Cheesegull
std::string shiro::config::direct::search_url = "https://storage.ripple.moe";
std::string shiro::config::direct::download_url = "https://storage.ripple.moe";
uint32_t shiro::config::direct::beatmaps_amount = 50;

void shiro::config::direct::parse() {
    if (config_file != nullptr) {
        LOG_F(INFO, "Re-parsing direct.toml file...");
    }

    try {
        config_file = cpptoml::parse_file("direct.toml");
    }
    catch (const cpptoml::parse_exception &ex) {
        CAPTURE_EXCEPTION(ex);
        ABORT_F("Failed to parse direct.toml file: {}.", ex.what());
    }

    enabled = config_file->get_qualified_as<bool>("direct.enabled").value_or(true);

    if (!enabled) {
        return;
    }

    provider = config_file->get_qualified_as<int32_t>("direct.provider").value_or(1);

    switch (provider) {
        case 0: {
            hanaru_url = config_file->get_qualified_as<std::string>("hanaru.url").value_or("https://mirror.yukime.ml");
            port = config_file->get_qualified_as<uint32_t>("hanaru.port").value_or(8090);
            break;
        }
        case 1: {
            base_url = config_file->get_qualified_as<std::string>("emulate.base_url").value_or("https://yukime.ml");
            mirror_url = config_file->get_qualified_as<std::string>("emulate.mirror_url").value_or("https://mirror.yukime.ml");
            break;
        }
        case 2: {
            api_key = config_file->get_qualified_as<std::string>("beatconnect.api_key").value_or("");
            break;
        }
        case 3: {
            search_url = config_file->get_qualified_as<std::string>("cheesegull.search_url").value_or("https://storage.ripple.moe");
            download_url = config_file->get_qualified_as<std::string>("cheesegull.download_url").value_or("https://storage.ripple.moe");
            beatmaps_amount = std::clamp(config_file->get_qualified_as<uint32_t>("cheesegull.beatmaps_amount").value_or(50), 1u, 100u);
            break;
        }
        default: {
            ABORT_F("Invalid direct mode provided in direct.toml: {}", provider);
        }
    }

    LOG_F(INFO, "Successfully parsed direct.toml.");
}
