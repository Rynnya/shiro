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

#include "../../../database/tables/score_table.hh"
#include "../../../logger/sentry_logger.hh"
#include "../../../replays/replay_manager.hh"
#include "../../../scores/score.hh"
#include "../../../scores/score_helper.hh"
#include "../../../users/user.hh"
#include "../../../utils/string_utils.hh"
#include "get_replay_route.hh"

void shiro::routes::web::get_replay::handle(const crow::request &request, crow::response &response) {
    response.set_header("Content-Type", "text/plain; charset=UTF-8");
    response.set_header("cho-server", "shiro (https://github.com/Rynnya/shiro)");

    char *username = request.url_params.get("u");
    char *md5sum = request.url_params.get("h");
    char *score_id = request.url_params.get("c");

    if (username == nullptr || md5sum == nullptr || score_id == nullptr) {
        response.code = 400;
        response.end();
        return;
    }

    std::shared_ptr<users::user> user = std::make_shared<users::user>(username);

    if (!user->init()) {
        response.code = 400;
        response.end();
        return;
    }

    if (!user->check_password(md5sum)) {
        response.code = 401;
        response.end();
        return;
    }

    int32_t id = 0;

    if (!utils::strings::evaluate(score_id, id)) {
        LOG_F(WARNING, "Unable to convert score id {} to int32_t.", score_id);
        CAPTURE_EXCEPTION(std::invalid_argument("Unable to cast score id to int32_t."));

        response.code = 500;
        response.end();

        return;
    }

    scores::score s = scores::helper::get_score(id);

    if (s.id == -1) {
        response.code = 404;
        response.end();
        return;
    }

    if (!replays::has_replay(s)) {
        response.code = 404;
        response.end();
        return;
    }

    io::buffer buffer(replays::get_replay(s));

    auto db = shiro::database::instance->pop();

    db(update(tables::scores_table).set(
        tables::scores_table.times_watched += 1
    ).where(tables::scores_table.id == s.id));

    response.set_header("Content-Type", "application/octet-stream");
    response.set_header("Content-Disposition", fmt::format("attachment; filename={}.osr", s.id));
    response.end(buffer.serialize());
}

void shiro::routes::web::get_replay::handle_full(const crow::request& request, crow::response& response) {
    response.set_header("Content-Type", "text/plain; charset=UTF-8");
    response.set_header("cho-server", "shiro (https://github.com/Rynnya/shiro)");

    char *score_id = request.url_params.get("id");

    if (score_id == nullptr) {
        response.code = 400;
        response.end("id required");
        return;
    }

    int32_t id = 0;

    if (!utils::strings::evaluate(score_id, id)) {
        LOG_F(WARNING, "Unable to convert score id {} to int32_t.", score_id);
        CAPTURE_EXCEPTION(std::invalid_argument("Unable to cast score id to int32_t."));

        response.code = 400;
        response.end("id must be integral");

        return;
    }

    scores::score s = scores::helper::get_score(id);
    std::string replay = replays::get_full_replay(s);

    if (replay.empty()) {
        response.code = 404;
        response.end("replay not found");
        return;
    }

    response.set_header("Content-Type", "application/octet-stream");
    response.set_header("Content-Disposition", fmt::format("attachment; filename={}.osr", s.id));
    response.end(replay);
}