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

#include "../utils/filesystem.hh"

#include <fstream>
#include <sstream>
#include <string>

#include "../beatmaps/beatmap_helper.hh"
#include "../config/score_submission_file.hh"
#include "../io/osu_buffer.hh"
#include "../scores/score_helper.hh"
#include "../thirdparty/naga.hh"
#include "../users/user_manager.hh"
#include "../utils/crypto.hh"
#include "../utils/osu_string.hh"
#include "../utils/time_utils.hh"
#include "replay.hh"
#include "replay_manager.hh"

static fs::path dir = fs::current_path() / "replays";

void shiro::replays::init() {
    if (!fs::exists(dir)) {
        fs::create_directories(dir);
    }
}

void shiro::replays::save_replay(const shiro::scores::score &s, const beatmaps::beatmap &beatmap, std::string&& replay) {
    if (!s.passed && !config::score_submission::save_failed_scores) {
        return;
    }

    if (!scores::helper::is_ranked(s, beatmap) && !config::score_submission::save_unranked_scores) {
        return;
    }

    fs::path filename = dir / std::string(std::to_string(s.id) + ".osr");
    std::shared_ptr<users::user> user = users::manager::get_user_by_id(s.user_id);

    if (user == nullptr) {
        return;
    }

    if (fs::exists(filename)) {
        fs::remove(filename);
    }

    std::ofstream stream(filename, std::ofstream::trunc | std::ofstream::binary);
    stream << replay;
    stream.close();
}

std::string shiro::replays::get_replay(const shiro::scores::score &s) {
    if (!has_replay(s)) {
        return "";
    }

    fs::path filename = dir / (std::to_string(s.id) + ".osr");
    std::stringstream result;
    std::ifstream stream = std::ifstream(filename, std::ifstream::binary);

    result << stream.rdbuf();
    stream.close();

    return result.str();
}

std::string shiro::replays::get_full_replay(const shiro::scores::score &s) {
    std::string username = users::manager::get_username_by_id(s.user_id);

    // Convert raw replay into full osu! replay file
    // Reference: https://osu.ppy.sh/help/wiki/osu!_File_Formats/Osr_(file_format)

    std::string raw_replay = get_replay(s);

    if (raw_replay.empty()) {
        return "";
    }

    // poot are you?
    const std::string hash_string = fmt::format(
        "{}p{}o{}o{}t{}a{}r{}e{}y{}o{}u{}{}{}",
        s.count_100 + s.count_300, s.count_50, s.count_gekis, s.count_katus, s.count_misses,
        s.beatmap_md5, s.max_combo, s.fc ? "True" : "False",
        username, s.total_score, s.rank, s.mods, "True"
    );

    std::string beatmap_md5 = utils::osu_string(s.beatmap_md5);
    std::string osu_username = utils::osu_string(username);
    std::string hash = utils::osu_string(utils::crypto::md5::hash(hash_string));

    io::buffer buffer;

    buffer.write<uint8_t>(s.play_mode);
    buffer.write<int32_t>(20210520);

    buffer.write<std::string>(beatmap_md5);
    buffer.write<std::string>(osu_username);
    buffer.write<std::string>(hash);

    buffer.write<int16_t>(s.count_300);
    buffer.write<int16_t>(s.count_100);
    buffer.write<int16_t>(s.count_50);
    buffer.write<int16_t>(s.count_gekis);
    buffer.write<int16_t>(s.count_katus);
    buffer.write<int16_t>(s.count_misses);

    buffer.write<int32_t>(s.total_score);
    buffer.write<int16_t>(s.max_combo);
    buffer.write<uint8_t>(s.fc);
    buffer.write<int32_t>(s.mods);

    buffer.write<uint8_t>(0);
    buffer.write<int64_t>(utils::time::get_ticks_from_time(s.time));

    buffer.write<int32_t>(raw_replay.size());
    buffer.write<std::string>(raw_replay);

    buffer.write<int64_t>(s.id);
    buffer.write<int32_t>(0);

    return buffer.serialize();
}

bool shiro::replays::has_replay(const shiro::scores::score &s) {
    return fs::exists(dir / (std::to_string(s.id) + ".osr"));
}
