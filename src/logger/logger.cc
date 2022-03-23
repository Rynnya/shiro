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
#include "../shiro.hh"
#include "logger.hh"

void shiro::logging::init() {
    naga::config config {};

    naga::reset_log_level(config, naga::log_level::info);
    config.filepath = "logs/shiro {:%Y-%m-%d %H-%M-%S}.log";

    static_cast<void>(naga::init(config));

    LOG_F(INFO, "Logging was successfully initialized.");
}
