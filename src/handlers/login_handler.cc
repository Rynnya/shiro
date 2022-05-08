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

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <cmath>

#include "../channels/channel_manager.hh"
#include "../config/bancho_file.hh"
#include "../config/ipc_file.hh"
#include "../config/score_submission_file.hh"
#include "../geoloc/geoloc.hh"
#include "../geoloc/location_info.hh"
#include "../io/layouts/packets.hh"
#include "../io/osu_writer.hh"
#include "../logger/sentry_logger.hh"
#include "../thirdparty/digestpp.hh"
#include "../thirdparty/naga.hh"
#include "../thirdparty/uuid.hh"
#include "../users/user.hh"
#include "../users/user_manager.hh"
#include "../users/user_punishments.hh"
#include "../utils/bot_utils.hh"
#include "../utils/login_responses.hh"
#include "../utils/osu_client.hh"
#include "../utils/string_utils.hh"
#include "../utils/time_utils.hh"
#include "login_handler.hh"

void shiro::handler::login::handle(const crow::request &request, crow::response &response) {
    if (request.body.empty()) {
        response.end();
        return;
    }

    std::vector<std::string> lines;
    boost::split(lines, request.body, boost::is_any_of("\n"));

    if (lines.size() != 4) {
        response.code = 403;
        response.end();

        LOG_F(WARNING, "Received invalid login request from {}: Login body has wrong length ({} != 4).", request.get_ip_address(), lines.size());
        return;
    }

    std::string username = lines.at(0);
    std::string password_md5 = lines.at(1);
    std::string infos = lines.at(2);

    std::vector<std::string> additional_info;
    boost::split(additional_info, infos, boost::is_any_of("|"));

    if (additional_info.size() != 5) {
        response.code = 403;
        response.end();

        LOG_F(WARNING, "Received invalid login request from {}: Additional info has wrong length.", request.get_ip_address());
        return;
    }

    io::osu_writer writer;
    writer.protocol_negotiation(io::cho_protocol);

    std::shared_ptr<users::user> user = std::make_shared<users::user>(username);

    if (!user->init()) {
        writer.login_reply(static_cast<int32_t>(utils::login_responses::invalid_credentials));

        response.end(writer.serialize());

        LOG_F(WARNING, "{} ({}) tried to login as non-existent user.", username, request.get_ip_address());
        return;
    }

    if (!user->check_password(password_md5)) {
        writer.login_reply(static_cast<int32_t>(utils::login_responses::invalid_credentials));

        response.end(writer.serialize());

        LOG_F(WARNING, "{} ({}) tried to login with wrong password.", username, request.get_ip_address());
        return;
    }

    if (users::punishments::is_banned(user->user_id)) {
        writer.login_reply(static_cast<int32_t>(utils::login_responses::user_banned));

        response.end(writer.serialize());

        LOG_F(WARNING, "{} ({}) tried to login while being banned.", username, request.get_ip_address());
        return;
    }

    if (user->permissions == 0) { // Read permissions (which is 1) available to everyone who registered successfully
        writer.login_reply(static_cast<int32_t>(utils::login_responses::verification_required));

        response.end(writer.serialize());

        LOG_F(WARNING, "{} ({}) tried to login while awaiting email verification.", username, request.get_ip_address());
        return;
    }

    std::string version = additional_info.at(0);
    std::string utc_offset = additional_info.at(1);
    std::string hwid = additional_info.at(3);
    int32_t build = 20131216;

    std::string parseable_version;
    std::string::size_type dot_index = version.find('.');

    if (dot_index != std::string::npos) {
        parseable_version = version.substr(1, dot_index - 1);
    }
    else {
        parseable_version = version;

        parseable_version.erase(std::remove_if(parseable_version.begin(), parseable_version.end(), 
            [](char c) { return !std::isdigit(c); }), parseable_version.end());
    }

    if (!utils::strings::evaluate(parseable_version, build)) {
        LOG_F(WARNING, "Unable to cast `{}` to int32_t.", version);
        CAPTURE_EXCEPTION(std::invalid_argument("Parseable version was invalid."));

        if (config::score_submission::restrict_mismatching_client_version) {
            writer.login_reply(static_cast<int32_t>(utils::login_responses::server_error));

            response.end(writer.serialize());
            return;
        }
    }

    user->client_type = utils::clients::parse_version(version, build);

    // The osu!stable client disallows certain actions for some very low user id's. In order to circumstance
    // this, disallow users with an user id of less than 10 (except if they're bots) to login.
    if (user->user_id < 10 && user->client_type != +utils::clients::osu_client::aschente) {
        writer.announce(fmt::format("Your account has an invalid user id assigned ({} < 10).", user->user_id));
        writer.login_reply(static_cast<int32_t>(utils::login_responses::invalid_credentials));

        response.end(writer.serialize());

        LOG_F(ERROR, "{} ({}) tried to login as user with id less than 10.", username, request.get_ip_address());
        return;
    }

    std::chrono::seconds seconds = utils::time::current_time();

    user->token = sole::uuid4().str();
    user->client_version = version;
    user->client_build = build;
    user->hwid = digestpp::sha256().absorb(hwid).hexdigest();
    user->last_ping = seconds;

    uint8_t time_zone = 9;
    int32_t parsed_time_zone = 0;

    if (utils::strings::evaluate(utc_offset, parsed_time_zone)) {
        time_zone = static_cast<uint8_t>(parsed_time_zone + 24);
    }

    geoloc::location_info location_info = geoloc::get_location(request.get_ip_address());

    user->presence.country_id = location_info.country;
    user->presence.latitude = location_info.latitude;
    user->presence.longitude = location_info.longitude;

    if (user->country == "XX") {
        user->update_country(location_info.country_str);
    }

    user->verify_address(request.get_ip_address());

    user->presence.time_zone = time_zone;

    users::manager::login_user(user);

    response.set_header("cho-token", user->token);

    writer.login_reply(user->user_id);
    writer.login_permissions(user->presence.permissions);

    // These are marked static as config reloading is not yet implemented
    static std::string alert = config::bancho::alert;
    static std::string title_image = config::bancho::title_image;
    static std::string title_url = config::bancho::title_url;

    if (!alert.empty()) {
        writer.announce(alert);
    }

    if (!title_image.empty() || !title_url.empty()) {
        writer.title_update(fmt::format("{}|{}", title_image, title_url));
    }

    if (request.raw_url.find("ppy.sh") != std::string::npos) {
        utils::bot::respond(fmt::format(
            "Seems like you still use old type of connection. Please, read about (-devserver)[{}] on our site.",
            config::ipc::frontend_url
        ), user, config::bot::name, true);
    }

    writer.friend_list(user->friends);

    writer.channel_listing_complete();
    channels::manager::write_channels(writer, user);
    channels::manager::auto_join(writer, user);

    io::osu_writer global_writer;
    global_writer.user_stats(user->stats);
    global_writer.user_presence(user->presence);

    // Give the user his own stats and presence as well as the bots' presence
    writer.user_stats(user->stats);
    writer.user_presence(user->presence);
    writer.user_presence(users::manager::get_user_by_id(1)->presence);

    if (users::punishments::is_silenced(user->user_id)) {
        auto [timestamp, duration] = users::punishments::get_silence_time(user->user_id);

        writer.user_ban_info((timestamp + duration) - seconds.count());
        global_writer.user_silenced(user->user_id);
    }

    if (user->hidden) {
        utils::bot::respond(fmt::format(
            "(Your account has been restricted)[{}]. "
            "Because of that, your profile has been hidden from the public. "
            "If you believe this is a mistake, (support contact support)[{}] "
            "to have your account status reviewed.",
            user->get_url(), config::ipc::frontend_url
        ), user, config::bot::name, true);
    }

    users::manager::iterate([user, &writer, &global_writer](std::shared_ptr<users::user>& online_user) {
        if (online_user == user) {
            return;
        }

        writer.user_presence(online_user->presence);
        writer.user_stats(online_user->stats);

        if (!user->hidden) {
            online_user->queue.enqueue(global_writer);
        }
    }, true);

    std::string result = writer.serialize();

    if (!user->queue.is_empty()) {
        result.append(user->queue.serialize());
    }

    response.end(result);
}
