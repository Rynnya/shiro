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

#include "../../../database/tables/user_table.hh"
#include "../../../beatmaps/beatmap.hh"
#include "../../../beatmaps/beatmap_helper.hh"
#include "../../../geoloc/country_ids.hh"
#include "../../../logger/sentry_logger.hh"
#include "../../../scores/score_helper.hh"
#include "../../../thirdparty/loguru.hh"
#include "../../../users/user_manager.hh"
#include "../../../utils/mods.hh"
#include "../../../utils/string_utils.hh"
#include "get_scores_route.hh"

void shiro::routes::web::get_scores::handle(const crow::request &request, crow::response &response) {
    response.set_header("Content-Type", "text/plain; charset=UTF-8");
    response.set_header("cho-server", "shiro (https://github.com/Marc3842h/shiro)");

    char *md5sum = request.url_params.get("c");
    char *beatmapset_id = request.url_params.get("i");
    char *type = request.url_params.get("v");

    char *username = request.url_params.get("us");
    char *hash = request.url_params.get("ha");

    if (md5sum == nullptr || beatmapset_id == nullptr || type == nullptr || username == nullptr || hash == nullptr) {
        response.code = 400;
        response.end();
        return;
    }

    std::shared_ptr<users::user> user = users::manager::get_user_by_username(username);

    if (user == nullptr) {
        response.code = 400;
        response.end();

        LOG_F(WARNING, "Received request for score listing from offline user.");
        return;
    }

    if (!user->check_password(hash)) {
        response.code = 403;
        response.end();

        LOG_F(WARNING, "Received request for score listing from %s with incorrect password.", user->presence.username.c_str());
        return;
    }

    bool parse_result = true;
    int32_t scoreboard_type = 0;
    beatmaps::beatmap beatmap;

    parse_result &= utils::strings::safe_int(beatmapset_id, beatmap.beatmapset_id);
    parse_result &= utils::strings::safe_int(type, scoreboard_type);
    beatmap.beatmap_md5 = md5sum;

    if (!parse_result)
    {
        LOG_F(ERROR, "Unable to convert sent values to beatmap metadata.");
        logging::sentry::exception(std::invalid_argument("Sent values was not a numbers."));

        response.code = 500;
        response.end();
        return;
    }

    beatmap.fetch();

    if (beatmap.ranked_status == static_cast<int32_t>(shiro::beatmaps::status::unsubmitted) && beatmap.exist())
    {
        response.end("1|false");
        return;
    }

    int32_t mods_list = static_cast<int32_t>(utils::mods::none);

    const tables::users users_table {};
    sqlpp::mysql::connection db(db_connection->get_config());
    bool is_relax = db(sqlpp::select(users_table.is_relax).from(users_table).where(users_table.id == user->user_id)).front().is_relax;

    std::vector<scores::score> score_list;

    switch (scoreboard_type) 
    {
        case 1: 
        {
            score_list = scores::helper::fetch_all_scores(md5sum, is_relax);
            break;
        }
        case 2: {
            char *mods = request.url_params.get("mods");

            if (mods == nullptr)
            {
                response.code = 400;
                response.end();
                return;
            }

            if (utils::strings::safe_int(mods, mods_list))
            {
                is_relax = (mods_list & static_cast<uint32_t>(utils::mods::relax));
                score_list = scores::helper::fetch_mod_scores(md5sum, mods_list, is_relax);
                break;
            }

            LOG_F(ERROR, "Unable to convert sent values (`%s`) to mods.", mods);
            logging::sentry::exception(std::invalid_argument("Mods was not a number."));

            response.code = 500;
            response.end();
            return;
        }
        case 3: {
            score_list = scores::helper::fetch_friend_scores(md5sum, user, is_relax);
            break;
        }
        case 4: {
            score_list = scores::helper::fetch_country_scores(md5sum, geoloc::get_country_id(user->country), is_relax);
            break;
        }
        default: {
            response.code = 400;
            response.end();
            return;
        }
    }

    std::string res = beatmap.build_header(score_list);

    if (beatmaps::helper::has_leaderboard(beatmaps::helper::fix_beatmap_status(beatmap.ranked_status))) {
        scores::score top_score_user = scores::helper::fetch_top_score_user(beatmap.beatmap_md5, user, is_relax);

        if (top_score_user.id == -1) {
            res.append("\n");
        } else {
            if (scoreboard_type == 2 && top_score_user.mods != mods_list) {
                res.append("\n");
            } else {
                res.append(top_score_user.to_string(score_list, is_relax));
            }
        }

        for (scores::score &submitted_score : score_list) {
            res.append(submitted_score.to_string(score_list, is_relax));
        }
    }

    response.end(res);
}
