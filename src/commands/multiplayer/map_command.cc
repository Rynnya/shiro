/*
 * shiro - High performance, high quality osu!Bancho C++ re-implementation
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

#include "../../beatmaps/beatmap.hh"
#include "../../beatmaps/beatmap_ranked_status.hh"
#include "../../multiplayer/match_manager.hh"
#include "../../utils/bot_utils.hh"
#include "../../utils/slot_status.hh"
#include "../../utils/string_utils.hh"
#include "map_command.hh"

bool shiro::commands_mp::map(std::deque<std::string>& args, std::shared_ptr<shiro::users::user> user, std::string channel) {
    if (args.size() < 1) {
        utils::bot::respond("Usage: !mp map <number>", user, channel, true);
        return true;
    }

    if (!shiro::multiplayer::match_manager::in_match(user)) {
        utils::bot::respond("You must be in room to perform this action!", user, channel, true);
        return true;
    }

    int32_t beatmap_id;
    if (!shiro::utils::strings::safe_int(args.at(0), beatmap_id)) {
        utils::bot::respond("You must provide a number!", user, channel, true);
        utils::bot::respond("Usage: !mp map <beatmap_id>", user, channel, true);
        return true;
    }

    shiro::beatmaps::beatmap beatmap;
    beatmap.beatmap_id = beatmap_id;
    beatmap.fetch();

    if (beatmap.ranked_status == static_cast<int32_t>(shiro::beatmaps::status::unknown)) {
        utils::bot::respond("Beatmap cannot be found in osu!API and our database, sorry :c", user, channel, true);
        return true;
    }

    shiro::multiplayer::match_manager::iterate([&user, &channel, beatmap](shiro::io::layouts::multiplayer_match& match) -> bool {
        auto iterator = std::find(match.multi_slot_id.begin(), match.multi_slot_id.end(), user->user_id);

        if (iterator == match.multi_slot_id.end()) {
            return false;
        }

        if (match.host_id == user->user_id) {
            bool changed = match.beatmap_id != beatmap.beatmap_id;

            match.beatmap_id = beatmap.beatmap_id;
            match.beatmap_name = beatmap.song_name;
            match.beatmap_checksum = beatmap.beatmap_md5;

            if (changed) {
                for (size_t i = 0; i < match.multi_slot_id.size(); i++) {
                    if (match.multi_slot_id.at(i) == -1) {
                        continue;
                    }

                    if (match.multi_slot_status.at(i) != static_cast<uint8_t>(utils::slot_status::not_ready)) {
                        continue;
                    }

                    match.multi_slot_status.at(i) = static_cast<uint8_t>(utils::slot_status::not_ready);
                }

                match.send_update(true);
                utils::bot::respond("Map was changed to " + std::to_string(beatmap.beatmap_id), user, channel, true);
                return true;
            }

            utils::bot::respond("Previous map id's and current are equals.", user, channel, true);
            return true;
        }

        utils::bot::respond("You must be a host to perform this action!", user, channel, true);
        return true;
    });

    return true;
}
