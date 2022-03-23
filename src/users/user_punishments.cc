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

#include <cinttypes>

#include "../channels/discord_webhook.hh"
#include "../config/ipc_file.hh"
#include "../database/tables/punishments_table.hh"
#include "../thirdparty/naga.hh"
#include "../utils/bot_utils.hh"
#include "../utils/login_responses.hh"
#include "../utils/punishment_type.hh"
#include "../utils/time_utils.hh"
#include "../shiro.hh"
#include "user_manager.hh"
#include "user_punishments.hh"

void shiro::users::punishments::init() {
    scheduler.Schedule(1min, [](tsc::TaskContext ctx) {
        auto db = shiro::database::instance->pop();
        auto result = db(select(all_of(tables::punishments_table)).from(tables::punishments_table).where(tables::punishments_table.active));

        if (result.empty()) {
            ctx.Repeat();
            return;
        }

        std::chrono::seconds seconds = utils::time::current_time();

        for (const auto &row : result) {
            if (row.duration.is_null()) {
                continue;
            }

            utils::punishment_type type = static_cast<utils::punishment_type>(row.type.value());
            int32_t timestamp = row.time;
            int32_t duration = row.duration;

            if (seconds.count() >= (timestamp + duration)) {
                db(update(tables::punishments_table).set(tables::punishments_table.active = false).where(tables::punishments_table.id == row.id));

                switch (type) {
                    case utils::punishment_type::silence: {
                        LOG_F(INFO, "User {} has been unsilenced automatically.", static_cast<int64_t>(row.id));
                        break;
                    }
                    case utils::punishment_type::restrict: {
                        std::shared_ptr<user> user = manager::get_user_by_id(row.id);

                        if (user != nullptr) {
                            io::osu_writer writer;

                            writer.announce("Your restriction has ended. Please login again.");
                            writer.login_reply(static_cast<int32_t>(utils::login_responses::account_password_reset));

                            user->queue.enqueue(writer);
                        }

                        LOG_F(INFO, "User {} has been unrestricted automatically.", static_cast<int64_t>(row.id));
                        break;
                    }
                    case utils::punishment_type::ban: {
                        LOG_F(INFO, "User {} has been unbanned automatically.", static_cast<int64_t>(row.id));
                        break;
                    }
                    default: {
                        break;
                    }
                }
            }
        }

        ctx.Repeat();
    });
}

void shiro::users::punishments::kick(int32_t user_id, int32_t origin, const std::string &reason) {
    std::chrono::seconds seconds = utils::time::current_time();

    auto db = shiro::database::instance->pop();

    db(insert_into(tables::punishments_table).set(
        tables::punishments_table.user_id = user_id,
        tables::punishments_table.origin_id = origin,
        tables::punishments_table.type = static_cast<uint16_t>(utils::punishment_type::kick),
        tables::punishments_table.time = seconds.count(),
        tables::punishments_table.active = false,
        tables::punishments_table.reason = reason
    ));

    std::shared_ptr<user> user = manager::get_user_by_id(user_id);
    std::string username = manager::get_username_by_id(user_id);
    std::string origin_username = manager::get_username_by_id(origin);

    LOG_F(INFO, "{} has been kicked for '{}' by {}.", username, reason, origin_username);

    if (user == nullptr) {
        return;
    }

    io::osu_writer writer;

    writer.announce(reason);
    writer.login_reply(static_cast<int32_t>(utils::login_responses::invalid_credentials));

    user->queue.enqueue(writer);
}

void shiro::users::punishments::silence(int32_t user_id, int32_t origin, uint32_t duration, const std::string &reason) {
    if (is_silenced(user_id)) {
        return;
    }

    std::chrono::seconds seconds = utils::time::current_time();
    auto db = shiro::database::instance->pop();

    db(insert_into(tables::punishments_table).set(
        tables::punishments_table.user_id = user_id,
        tables::punishments_table.origin_id = origin,
        tables::punishments_table.type = static_cast<uint16_t>(utils::punishment_type::silence),
        tables::punishments_table.time = seconds.count(),
        tables::punishments_table.duration = duration,
        tables::punishments_table.active = true,
        tables::punishments_table.reason = reason
    ));

    std::shared_ptr<user> user = manager::get_user_by_id(user_id);
    std::string username = manager::get_username_by_id(user_id);
    std::string origin_username = manager::get_username_by_id(origin);

    LOG_F(INFO, "{} has been silenced for {} seconds ({} minutes)  for '{}' by {}.", username, duration, duration / 60, reason, origin_username);

    shiro::channels::discord_webhook::send_silence_message(username, origin_username, reason, duration);

    if (user == nullptr) {
        return;
    }

    io::osu_writer global_writer;
    global_writer.user_silenced(user_id);

    users::manager::iterate([user, &global_writer](std::shared_ptr<users::user> online_user) {
        online_user->queue.enqueue(global_writer);
    }, true);

    utils::bot::respond(
            fmt::format("You have been silenced for {} seconds for '{}'.", duration, reason),
            user, config::bot::name, true
    );

    io::osu_writer writer;
    writer.user_ban_info(duration); // This will lock the client

    user->queue.enqueue(writer);
}

void shiro::users::punishments::restrict(int32_t user_id, int32_t origin, const std::string &reason) {
    if (is_restricted(user_id)) {
        return;
    }

    std::chrono::seconds seconds = utils::time::current_time();
    auto db = shiro::database::instance->pop();

    db(insert_into(tables::punishments_table).set(
        tables::punishments_table.user_id = user_id,
        tables::punishments_table.origin_id = origin,
        tables::punishments_table.type = static_cast<uint16_t>(utils::punishment_type::restrict),
        tables::punishments_table.time = seconds.count(),
        tables::punishments_table.active = true,
        tables::punishments_table.reason = reason
    ));

    std::shared_ptr<user> user = manager::get_user_by_id(user_id);
    std::string username = manager::get_username_by_id(user_id);
    std::string origin_username = manager::get_username_by_id(origin);

    LOG_F(INFO, "{} has been restricted for '{}' by {}.", username, reason, origin_username);

    shiro::channels::discord_webhook::send_restrict_message(username, origin_username, reason);

    if (user == nullptr) {
        return;
    }

    user->hidden = true;

    utils::bot::respond(fmt::format(
            "[{} Your account has been restricted]. "
            "Because of that, your profile has been hidden from the public. "
            "If you believe this is a mistake, [{} support contact support] "
            "to have your account status reviewed.", 
        user->get_url(), config::ipc::frontend_url
    ),  user, config::bot::name, true
    );

    io::layouts::user_quit quit;
    io::osu_writer writer;

    quit.user_id = user_id;
    quit.state = 0;

    writer.user_quit(quit);

    users::manager::iterate([user_id, &writer](std::shared_ptr<users::user> online_user) {
        if (online_user->user_id == user_id) {
            return;
        }

        online_user->queue.enqueue(writer);
    });
}

void shiro::users::punishments::ban(int32_t user_id, int32_t origin, const std::string &reason) {
    if (is_banned(user_id)) {
        return;
    }

    std::chrono::seconds seconds = utils::time::current_time();
    auto db = shiro::database::instance->pop();

    db(insert_into(tables::punishments_table).set(
        tables::punishments_table.user_id = user_id,
        tables::punishments_table.origin_id = origin,
        tables::punishments_table.type = static_cast<uint16_t>(utils::punishment_type::ban),
        tables::punishments_table.time = seconds.count(),
        tables::punishments_table.active = true,
        tables::punishments_table.reason = reason
    ));

    std::shared_ptr<user> user = manager::get_user_by_id(user_id);
    std::string username = manager::get_username_by_id(user_id);
    std::string origin_username = manager::get_username_by_id(origin);

    LOG_F(INFO, "{} has been banned for '{}' by {}.", username, reason, origin_username);

    shiro::channels::discord_webhook::send_ban_message(username, origin_username, reason);

    if (user == nullptr) {
        return;
    }

    io::osu_writer writer;

    writer.announce(reason);
    writer.login_reply(static_cast<int32_t>(utils::login_responses::user_banned)); // This will lock the client

    user->queue.enqueue(writer);
}

bool shiro::users::punishments::is_silenced(int32_t user_id) {
    auto db = shiro::database::instance->pop();

    auto result = db(select(all_of(tables::punishments_table)).from(tables::punishments_table).where(
        tables::punishments_table.user_id == user_id and
        tables::punishments_table.type == static_cast<uint16_t>(utils::punishment_type::silence) and
        tables::punishments_table.active == true
    ).limit(1u));

    return !result.empty();
}

bool shiro::users::punishments::is_restricted(int32_t user_id) {
    auto db = shiro::database::instance->pop();

    auto result = db(select(all_of(tables::punishments_table)).from(tables::punishments_table).where(
        tables::punishments_table.user_id == user_id and
        tables::punishments_table.type == static_cast<uint16_t>(utils::punishment_type::restrict) and
        tables::punishments_table.active == true
    ).limit(1u));

    return !result.empty();
}

bool shiro::users::punishments::is_banned(int32_t user_id) {
    auto db = shiro::database::instance->pop();

    auto result = db(select(all_of(tables::punishments_table)).from(tables::punishments_table).where(
        tables::punishments_table.user_id == user_id and
        tables::punishments_table.type == static_cast<uint16_t>(utils::punishment_type::ban) and
        tables::punishments_table.active == true
    ).limit(1u));

    return !result.empty();
}

bool shiro::users::punishments::can_chat(int32_t user_id) {
    std::shared_ptr<user> user = manager::get_user_by_id(user_id);

    if (user == nullptr) {
        return false;
    }

    if (user->hidden) {
        return false;
    }

    return !is_silenced(user_id);
}

bool shiro::users::punishments::has_scores(int32_t user_id) {
    auto db = shiro::database::instance->pop();

    auto result = db(select(all_of(tables::punishments_table)).from(tables::punishments_table).where(
        tables::punishments_table.user_id == user_id and
        (tables::punishments_table.type == static_cast<uint16_t>(utils::punishment_type::ban) or
        tables::punishments_table.type == static_cast<uint16_t>(utils::punishment_type::restrict)) and
        tables::punishments_table.active == true
    ).limit(1u));

    return result.empty();
}

std::tuple<int32_t, uint32_t> shiro::users::punishments::get_silence_time(int32_t user_id) {
    if (!is_silenced(user_id)) {
        return {};
    }

    auto db = shiro::database::instance->pop();

    auto result = db(select(all_of(tables::punishments_table)).from(tables::punishments_table).where(
        tables::punishments_table.user_id == user_id and
        tables::punishments_table.type == static_cast<uint16_t>(utils::punishment_type::silence) and
        tables::punishments_table.active == true
    ).limit(1u));

    if (result.empty()) {
        return {};
    }

    const auto &row = result.front();

    return std::make_pair(row.time, row.duration);
}
