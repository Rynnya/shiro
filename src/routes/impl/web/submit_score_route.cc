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

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <functional>
#include <memory>

#include "../../../beatmaps/beatmap.hh"
#include "../../../beatmaps/beatmap_helper.hh"
#include "../../../channels/discord_webhook.hh"
#include "../../../config/score_submission_file.hh"
#include "../../../database/tables/score_table.hh"
#include "../../../logger/sentry_logger.hh"
#include "../../../pp/pp_score_metric.hh"
#include "../../../ranking/ranking_helper.hh"
#include "../../../replays/replay_manager.hh"
#include "../../../scores/score.hh"
#include "../../../scores/score_helper.hh"
#include "../../../scores/table_display.hh"
#include "../../../thirdparty/loguru.hh"
#include "../../../users/user.hh"
#include "../../../users/user_manager.hh"
#include "../../../users/user_punishments.hh"
#include "../../../utils/bot_utils.hh"
#include "../../../utils/crypto.hh"
#include "../../../utils/multipart_parser.hh"
#include "../../../utils/string_utils.hh"
#include "../../../utils/time_utils.hh"
#include "submit_score_route.hh"

void shiro::routes::web::submit_score::handle(const crow::request &request, crow::response &response) {
    response.set_header("Content-Type", "text/plain; charset=UTF-8");
    response.set_header("cho-server", "shiro (https://github.com/Marc3842h/shiro)");

    const std::string &user_agent = request.get_header_value("user-agent");

    if (user_agent.empty() || user_agent != "osu!") {
        response.end();

        LOG_F(WARNING, "Received score submission from %s without osu! user agent.", request.get_ip_address().c_str());
        return;
    }

    const std::string &content_type = request.get_header_value("content-type");

    if (content_type.empty()) {
        response.end("error: invalid");

        LOG_F(ERROR, "Received score submission without content type.");
        return;
    }

    std::unique_ptr<utils::multipart_parser> parser = std::make_unique<utils::multipart_parser>(request.body, content_type);

    if (parser == nullptr)
        return;

    utils::multipart_form_fields fields = parser->parse_fields();
    std::string key = "h89f2-890h2h89b34g-h80g134n90133";

    if (fields.find("pass") == fields.end()) {
        response.end("error: pass");

        LOG_F(WARNING, "Received score submission without password.");
        return;
    }

    if (fields.find("iv") == fields.end()) {
        response.end("error: invalid");

        LOG_F(WARNING, "Received score without initialization vector.");
        return;
    }

    if (fields.find("score") == fields.end()) {
        response.end("error: invalid");

        LOG_F(WARNING, "Received score without score data.");
        return;
    }

    if (fields.find("osuver") != fields.end())
        key = "osu!-scoreburgr---------" + fields.at("osuver").body;

    std::vector<unsigned char> decrypted = utils::crypto::rijndael256::decode(
            utils::crypto::base64::decode(fields.at("iv").body.c_str()),
            key,
            utils::crypto::base64::decode(fields.at("score").body.c_str())
    );

    if (decrypted.empty()) {
        response.end("error: invalid");

        LOG_F(WARNING, "Received score without score metadata.");
        return;
    }

    std::string result(reinterpret_cast<char*>(&decrypted[0]), decrypted.size());

    std::vector<std::string> score_metadata;
    boost::split(score_metadata, result, boost::is_any_of(":"));

    if (score_metadata.size() < 18) {
        response.end("error: invalid");

        LOG_F(WARNING, "Received invalid score submission, score metadata doesn't have 18 or more parts.");
        return;
    }

    boost::trim_right(score_metadata.at(1));

    std::shared_ptr<users::user> user = users::manager::get_user_by_username(score_metadata.at(1));

    // This only occurs when the server restarted and osu submitted before being re-logged in
    if (user == nullptr) {
        response.end();

        LOG_F(WARNING, "Received score submission from offline user.");
        return;
    }

    if (!user->check_password(fields.at("pass").body)) {
        response.end("error: pass");

        LOG_F(WARNING, "Received score submission from %s with incorrect password.", user->presence.username.c_str());
        return;
    }

    if (users::punishments::is_banned(user->user_id)) {
        response.end("error: no");

        LOG_F(WARNING, "Received score submission from %s while user is banned.", user->presence.username.c_str());
        return;
    }

    sqlpp::mysql::connection db(db_connection->get_config());
    const tables::scores score_table {};

    int32_t game_version = 20131216;

    score_metadata.at(17).erase(std::remove_if(score_metadata.at(17).begin(), score_metadata.at(17).end(),
        [](char c) { return !std::isdigit(c); }), score_metadata.at(17).end());

    scores::score score;
    score.user_id = user->user_id;

    score.beatmap_md5 = score_metadata.at(0);
    score.hash = score_metadata.at(2);

    /* This way faster than lexical_cast, but provides less information why this thing crashed... */
    /* I run some tests and lexical_cast x100 takes around 9600 ns, when safe_* x100 takes around 3100 ns */
    /* If you have any problems - please debug program to catch real exception (bad_lexical_cast don't give any information) */

    bool parse_result = true;

    parse_result &= utils::strings::safe_int  (score_metadata.at(3),  score.count_300);
    parse_result &= utils::strings::safe_int  (score_metadata.at(4),  score.count_100);
    parse_result &= utils::strings::safe_int  (score_metadata.at(5),  score.count_50);
    parse_result &= utils::strings::safe_int  (score_metadata.at(6),  score.count_gekis);
    parse_result &= utils::strings::safe_int  (score_metadata.at(7),  score.count_katus);
    parse_result &= utils::strings::safe_int  (score_metadata.at(8),  score.count_misses);
    parse_result &= utils::strings::safe_ll   (score_metadata.at(9),  score.total_score);
    parse_result &= utils::strings::safe_int  (score_metadata.at(10), score.max_combo);
    parse_result &= utils::strings::safe_int  (score_metadata.at(13), score.mods);
    parse_result &= utils::strings::safe_uchar(score_metadata.at(15), score.play_mode);

    if (!parse_result)
    {
        logging::sentry::exception(std::invalid_argument("Score submission arguments was invalid."));

        response.end("error: invalid");

        LOG_F(WARNING, "Received score submission from %s with invalid types.", user->presence.username.c_str());
        return;
    }

    parse_result &= utils::strings::safe_int(score_metadata.at(17), game_version);

    if (!parse_result)
    {
        logging::sentry::exception(std::invalid_argument("Cannot convert game version into int32_t."));
        LOG_F(WARNING, "Unable to convert %s to game version.", score_metadata.at(17).c_str());

        // Give the client a chance to resubmit so the player doesn't get restricted for a fail on our side.
        if (config::score_submission::restrict_mismatching_client_version)
        {
            response.end("error: invalid");
            return;
        }
    }

    int32_t failtime = 0;
    bool failed = false;
    if (fields.find("ft") != fields.end())
    {
        failtime = utils::strings::safe_int(fields.at("ft").body);
        failed = failtime > 0;
    }

    if (score.play_mode > 3)
    {
        response.end("error: invalid");

        LOG_F(WARNING, "%s submitted a score with a invalid play mode.", user->presence.username.c_str());
        return;
    }

    std::chrono::seconds seconds = utils::time::current_time();

    score.rank = score_metadata.at(12);
    score.fc = utils::strings::to_bool(score_metadata.at(11));
    score.passed = utils::strings::to_bool(score_metadata.at(14));
    score.is_relax = score.mods & static_cast<uint32_t>(utils::mods::relax);
    score.time = seconds.count();

    score.accuracy = scores::helper::calculate_accuracy(
        static_cast<utils::play_mode>(score.play_mode),
        score.count_300, score.count_100, score.count_50,
        score.count_gekis, score.count_katus, score.count_misses
    );

    auto db_result = db(select(all_of(score_table)).from(score_table).where(score_table.hash == score.hash).limit(1u));

    // Score has already been submitted
    if (!db_result.empty()) {
        response.end("error: dup");

        LOG_F(WARNING, "%s resubmitted a previously submitted score.", user->presence.username.c_str());
        return;
    }

    // Switch between Relax and Classic
    user->update(score.is_relax);

    user->stats.play_count++;
    user->stats.total_hits
        += static_cast<int64_t>(score.count_300)
        +  static_cast<int64_t>(score.count_100)
        +  static_cast<int64_t>(score.count_50);

    beatmaps::beatmap beatmap;
    beatmap.beatmap_md5 = score.beatmap_md5;

    beatmap.fetch();

    if (score.passed)
        beatmap.pass_count++;

    beatmap.play_count++;
    beatmap.update_play_metadata();

    score.play_time = beatmap.hit_length;
    if (failed)
    {
        int32_t time = beatmap.hit_length;
        if (failtime / 1000 < beatmap.hit_length * 1.33)
            time = failtime / 1000;

        score.play_time = shiro::utils::time::adjusted_seconds(score.mods, time);
    }

    user->stats.play_time += score.play_time;

    if (fields.find("replay-bin") == fields.end()) {
        response.code = 400;
        response.end("error: invalid");

        if (config::score_submission::restrict_no_replay)
            users::punishments::restrict(user->user_id, 1, "No replay sent on score submission");

        LOG_F(WARNING, "Received score without replay data.");
        return;
    }

    if (beatmaps::helper::awards_pp(beatmaps::helper::fix_beatmap_status(beatmap.ranked_status)))
        score.pp = pp::calculate(beatmap, score);

    std::vector<scores::score> previous_scores = scores::helper::fetch_user_scores(beatmap.beatmap_md5, user, score.is_relax);
    bool overwrite = score.passed;

    // User has previous scores on this map and this also pass, enable overwriting mode
    if (!previous_scores.empty() && overwrite)
    {
        for (const scores::score &s : previous_scores)
        {
            double factor_score;
            double factor_iterator;

            // In relax only pp
            if (score.is_relax || !user->preferences.is_overwrite(static_cast<shiro::utils::play_mode>(score.play_mode)))
            {
                factor_score = score.pp;
                factor_iterator = s.pp;
            }
            else
            {
                factor_score = score.total_score;
                factor_iterator = s.total_score;
            }

            if (factor_iterator > factor_score)
            {
                overwrite = false;
                break;
            }
        }
    }

    // Auto restriction for weird things enabled in score_submission.toml
    auto [flagged, reason] = scores::helper::is_flagged(score, beatmap);

    if (flagged)
        users::punishments::restrict(user->user_id, 1, reason);

    // Auto restriction for bad replay submitters that submit without editing username
    if (config::score_submission::restrict_mismatching_username && score_metadata.at(1) != user->presence.username)
        users::punishments::restrict(user->user_id, 1, "Mismatching username on score submission (" + score_metadata.at(1) + " != " + user->presence.username + ")");

    // ...or the client build
    if (config::score_submission::restrict_mismatching_client_version && game_version != user->client_build)
        users::punishments::restrict(user->user_id, 1, "Mismatching client version on score submission (" + std::to_string(game_version) + " != " + std::to_string(user->client_build) + ")");

    // Auto restriction for notepad hack
    if (config::score_submission::restrict_notepad_hack && fields.find("bmk") != fields.end() && fields.find("bml") != fields.end()) {
        std::string bmk = fields.at("bmk").body;
        std::string bml = fields.at("bml").body;

        if (bmk != bml)
            users::punishments::restrict(user->user_id, 1, "Mismatching bmk and bml (notepad hack, " + bmk + " != " + bml + ")");
    }

    // Legacy table display (<20181221.4)
    bool legacy = request.url == "/web/osu-submit-modular.php";

    std::unique_ptr<scores::table_display> display = std::make_unique<scores::table_display>(user, beatmap, score, legacy);
    display->init();

    score.id = db(insert_into(score_table).set(
            score_table.user_id = score.user_id,
            score_table.hash = score.hash,
            score_table.beatmap_md5 = score.beatmap_md5,
            score_table.ranking = score.rank,
            score_table.score = score.total_score,
            score_table.max_combo = score.max_combo,
            score_table.pp = score.pp,
            score_table.accuracy = score.accuracy,
            score_table.mods = score.mods,
            score_table.full_combo = score.fc,
            score_table.completed = score.passed,
            score_table.count_300 = score.count_300,
            score_table.count_100 = score.count_100,
            score_table.count_50 = score.count_50,
            score_table.count_katus = score.count_katus,
            score_table.count_gekis = score.count_gekis,
            score_table.count_misses = score.count_misses,
            score_table.play_mode = score.play_mode,
            score_table.time = score.time,
            score_table.play_time = score.play_time,
            score_table.is_relax = score.is_relax
    ));

    if (overwrite)
    {
        user->stats.total_score += score.total_score;
        user->update_counts(score.rank);

        if (score.max_combo > user->stats.max_combo)
            user->stats.max_combo = score.max_combo;
    }

    replays::save_replay(score, beatmap, fields.at("replay-bin").body);

    if (!score.passed || !scores::helper::is_ranked(score, beatmap))
    {
        // We need save stats to keep play_time, counts and total_hits, also fixes 'sending statistics...' bug
        user->save_stats(score.is_relax);
        display->set_scoreboard_position(0);
        response.end(display->build());
        return;
    }

    scores::score top_score = scores::helper::fetch_top_score_user(beatmap.beatmap_md5, user, score.is_relax);
    std::vector<scores::score> first_scores = scores::helper::fetch_all_scores(beatmap.beatmap_md5, score.is_relax, 5);
    int32_t scoreboard_position = scores::helper::get_scoreboard_position(top_score, first_scores);

    if (top_score.hash == score.hash && !user->hidden) {
        if (scoreboard_position == 1) {
            char buffer[1024];
            std::snprintf(
                    buffer, sizeof(buffer),
                    "[%s %s] achieved rank #1 on [%s %s] (%s)",
                    user->get_url().c_str(), user->presence.username.c_str(),
                    beatmap.get_url().c_str(), beatmap.song_name.c_str(),
                    utils::play_mode_to_string(static_cast<utils::play_mode>(score.play_mode)).c_str()
            );

            utils::bot::respond(buffer, user, "#announce", false);

            std::thread([](sqlpp::mysql::connection& db, const std::shared_ptr<users::user>& user, const beatmaps::beatmap& beatmap, const scores::score& score)
            {
                shiro::channels::discord_webhook::send_top1_message(user, beatmap, score);

                const tables::scores_first scores_first_table {};
                auto exist = db(sqlpp::select(scores_first_table.beatmap_md5).from(scores_first_table).where(scores_first_table.beatmap_md5 == beatmap.beatmap_md5).limit(1u));
                if (exist.empty())
                {
                    db(sqlpp::insert_into(scores_first_table).set(
                        scores_first_table.score_id = score.id,
                        scores_first_table.beatmap_md5 = beatmap.beatmap_md5,
                        scores_first_table.user_id = score.user_id,
                        scores_first_table.play_mode = score.play_mode,
                        scores_first_table.is_relax = score.is_relax
                    ));
                    return;
                }

                // One result exist, we need to pop it
                exist.pop_front();
                db(sqlpp::update(scores_first_table).set(
                    scores_first_table.score_id = score.id,
                    scores_first_table.beatmap_md5 = beatmap.beatmap_md5,
                    scores_first_table.user_id = score.user_id,
                    scores_first_table.play_mode = score.play_mode,
                    scores_first_table.is_relax = score.is_relax
                ).where(scores_first_table.beatmap_md5 == beatmap.beatmap_md5));
            }, std::ref(db) /* We don't manipulate with database anymore */, std::ref(user), std::ref(beatmap), std::ref(score)).detach();
        }
    }

    display->set_scoreboard_position(scoreboard_position);

    if (overwrite)
        user->stats.ranked_score += score.total_score;

    user->stats.recalculate_pp(score.is_relax);
    user->stats.recalculate_accuracy(score.is_relax);

    user->save_stats(score.is_relax);

    if (overwrite && !user->hidden)
        ranking::helper::recalculate_ranks(static_cast<utils::play_mode>(score.play_mode), score.is_relax);

    response.end(display->build());
}
