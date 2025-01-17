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

#include <fstream>

#include "../thirdparty/naga.hh"
#include "../utils/curler.hh"
#include "../utils/filesystem.hh"
#include "../utils/string_utils.hh"
#include "beatmap_helper.hh"

static fs::path dir = fs::current_path() / "maps";

void shiro::beatmaps::helper::init() {
    if (!fs::exists(dir)) {
        fs::create_directories(dir);
    }
}

int32_t shiro::beatmaps::helper::fix_beatmap_status(int32_t status_code) {
    if (status_code == static_cast<int32_t>(status::needs_update)) {
        return static_cast<int32_t>(status::ranked);
    }

    if (status_code == static_cast<int32_t>(status::qualified)) {
        return static_cast<int32_t>(status::loved);
    }

    return status_code;
}

bool shiro::beatmaps::helper::has_leaderboard(int32_t status_code) {
    return status_code == static_cast<int32_t>(status::ranked)      ||
           status_code == static_cast<int32_t>(status::loved)       ||
           status_code == static_cast<int32_t>(status::qualified)   ||
           status_code == static_cast<int32_t>(status::approved);
}

bool shiro::beatmaps::helper::awards_pp(int32_t status_code) {
    return status_code == static_cast<int32_t>(status::ranked);
}

std::optional<std::string> shiro::beatmaps::helper::get_location(int32_t beatmap_id, bool download) {
    using fmt::format;
    const fs::path filename = dir / format("{}.osu", beatmap_id);

    if (fs::exists(filename)) {
        return filename.u8string();
    }

    if (!download) {
        return std::nullopt;
    }

    auto [success, output] = utils::curl::get(format("https://old.ppy.sh/osu/{}", beatmap_id));

    if (!success || output.empty()) {
        LOG_F(ERROR, "Unable to connect to osu! api: {}.", output);
        return std::nullopt;
    }

    std::ofstream stream(filename, std::ofstream::binary);
    stream << output;
    stream.close();

    return filename.u8string();
}

float shiro::beatmaps::helper::score_to_difficulty(beatmaps::beatmap beatmap, utils::play_mode mode) {
    switch (mode) {
        case utils::play_mode::taiko: {
            return beatmap.difficulty_taiko;
        }
        case utils::play_mode::fruits: {
            return beatmap.difficulty_ctb;
        }
        case utils::play_mode::mania: {
            return beatmap.difficulty_mania;
        }
        case utils::play_mode::standard:
        default: {
            return beatmap.difficulty_std;
        }
    }
}

std::string shiro::beatmaps::helper::build_difficulty_header(beatmaps::beatmap beatmap, utils::play_mode mode) {
    switch (mode) {
        case utils::play_mode::taiko: {
            return fmt::format("OD {0:.1f} | HP {1:.1f}", beatmap.od, beatmap.hp);
        }
        case utils::play_mode::mania: {
            return fmt::format("Keys {0:.1f} | OD {1:.1f} | HP {2:.1f}", beatmap.cs, beatmap.od, beatmap.hp);
        }
        case utils::play_mode::fruits:
        case utils::play_mode::standard:
        default: {
            return fmt::format("CS {0:.1f} | AR {1:.1f} | OD {2:.1f} | HP {3:.1f}", beatmap.cs, beatmap.ar, beatmap.od, beatmap.hp);
        }
    }
}
