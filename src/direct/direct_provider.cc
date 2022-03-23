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

#include "../config/direct_file.hh"
#include "../logger/sentry_logger.hh"

#include "providers/beatconnect.hh"
#include "providers/cheesegull.hh"
#include "providers/emulation.hh"
#include "providers/hanaru.hh"
#include "direct_provider.hh"

std::shared_ptr<shiro::direct::direct_provider> shiro::direct::provider = nullptr;

void shiro::direct::init() {
    if (!config::direct::enabled) {
        return;
    }

    switch (config::direct::provider) {
        case 0: {
            // Localhost Hanaru via websocket
            provider = std::make_shared<hanaru>();
            break;
        }
        case 1: {
            // Client request emulation
            provider = std::make_shared<emulation>();
            break;
        }
        case 2: {
            // Beatconnect
            provider = std::make_shared<beatconnect>();
            break;
        }
        case 3: {
            // Cheesegull
            provider = std::make_shared<cheesegull>();
            break;
        }
        default: {
            LOG_F(ERROR, "Invalid direct mode has been provided in bancho.toml: {}.", config::direct::provider);
            break;
        }
    }
}
