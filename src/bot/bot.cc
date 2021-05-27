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

#include <functional>
#include <unordered_map>
#include <utility>
#include <memory>

#include "../commands/public/help_command.hh"
#include "../commands/public/localclear_command.hh"
#include "../commands/public/roll_command.hh"
#include "../commands/public/switch_command.hh"
#include "../commands/staff/announce_command.hh"
#include "../commands/staff/ban_command.hh"
#include "../commands/staff/clear_command.hh"
#include "../commands/staff/clients_command.hh"
#include "../commands/staff/kick_command.hh"
#include "../commands/staff/recalculate_ranks.hh"
#include "../commands/staff/restart_command.hh"
#include "../commands/staff/restrict_command.hh"
#include "../commands/staff/rtx_command.hh"
#include "../commands/staff/silence_command.hh"
#include "../config/bot_file.hh"
#include "../config/db_file.hh"
#include "../database/tables/user_table.hh"
#include "../geoloc/country_ids.hh"
#include "../shiro.hh"
#include "../thirdparty/digestpp.hh"
#include "../thirdparty/loguru.hh"
#include "../thirdparty/uuid.hh"
#include "../users/user_manager.hh"
#include "../utils/escaper.hh"
#include "../utils/osu_client.hh"
#include "bot.hh"

std::shared_ptr<shiro::users::user> shiro::bot::bot_user = nullptr;
static std::unordered_map<std::string, std::function<bool(std::deque<std::string>&, std::shared_ptr<shiro::users::user>, std::string)>> commands_map;

void shiro::bot::init() {
    sqlpp::mysql::connection db(db_connection->get_config());
    const tables::users user_table{};
    const tables::users_stats users_stats_table{};
    const tables::users_stats_relax users_stats_relax_table {};

    auto result1 = db(select(all_of(user_table)).from(user_table).where(user_table.id == 1).limit(1u));

    // Check if the bot user exists, if not insert it into the db
    if (result1.empty())
        db(insert_into(user_table).set(
                user_table.id = 1,
                user_table.username = config::bot::name,
                user_table.safe_username = utils::escaper::make_safe(config::bot::name),
                user_table.password_md5 = digestpp::sha256().absorb(config::database::password).hexdigest(),
                user_table.salt = config::database::database,
                user_table.email = config::bot::name + "@shiro.host",
                user_table.ip = "127.0.0.1",
                user_table.registration_date = 0,
                user_table.latest_activity = 0,
                user_table.followers = 0,
                user_table.roles = 0xDEADCAFE, // Special role for robots
                user_table.userpage = "beep boop",
                user_table.country = "JP"
        ));

    auto result2 = db(select(all_of(users_stats_table)).from(users_stats_table).where(users_stats_table.id == 1).limit(1u));

    if (result2.empty())
        db(insert_into(users_stats_table).set(users_stats_table.id = 1));

    auto result3 = db(select(all_of(users_stats_relax_table)).from(users_stats_relax_table).where(users_stats_relax_table.id == 1).limit(1u));

    if (result3.empty())
        db(insert_into(users_stats_relax_table).set(users_stats_relax_table.id = 1));

    std::shared_ptr<users::user> bot_user = std::make_shared<users::user>(1);

    if (!bot_user->init())
        ABORT_F("Unable to initialize chat bot.");

    std::chrono::seconds seconds = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
    );

    bot_user->token = sole::uuid4().str();
    bot_user->hwid = digestpp::sha256().absorb(config::bot::name).hexdigest();
    bot_user->last_ping = seconds;

    bot_user->client_version = "b19700101.01";
    bot_user->client_build = 19700101;
    bot_user->client_type = utils::clients::osu_client::aschente;

    bot_user->presence.country_id = 111;
    bot_user->presence.time_zone = 9;
    bot_user->presence.latitude = 35.6895f;
    bot_user->presence.longitude = 139.6917f;

    users::manager::login_user(bot_user);

    bot::bot_user = bot_user;

    scheduler.Schedule(30s, [bot_user](tsc::TaskContext ctx) {
        bot_user->queue.clear();

        ctx.Repeat(30s);
    });

    LOG_F(INFO, "Bot has been successfully registered as %s and is now online.", config::bot::name.c_str());
}

void shiro::bot::init_commands() {
    commands_map.insert(std::make_pair("announce", commands::announce));
    commands_map.insert(std::make_pair("ban", commands::ban));
    commands_map.insert(std::make_pair("classic", commands::classic));
    commands_map.insert(std::make_pair("clear", commands::clear));
    commands_map.insert(std::make_pair("clients", commands::clients));
    commands_map.insert(std::make_pair("help", commands::help));
    commands_map.insert(std::make_pair("kick", commands::kick));
    commands_map.insert(std::make_pair("localclear", commands::localclear));
    commands_map.insert(std::make_pair("recalculate", commands::recalculate));
    commands_map.insert(std::make_pair("relax", commands::relax));
    commands_map.insert(std::make_pair("restart", commands::restart));
    commands_map.insert(std::make_pair("restrict", commands::restrict));
    commands_map.insert(std::make_pair("roll", commands::roll));
    commands_map.insert(std::make_pair("rtx", commands::rtx));
    commands_map.insert(std::make_pair("silence", commands::silence));

    LOG_F(INFO, "Bot commands have been successfully loaded. %lu commands available.", commands_map.size());
}

bool shiro::bot::handle(const std::string &command, std::deque<std::string> &args, std::shared_ptr<shiro::users::user> user, std::string channel) {
    try {
        return commands_map.at(command)(args, user, channel);
    } catch (const std::out_of_range &ex) {
        io::layouts::message msg;
        io::osu_writer writer;

        msg.sender = config::bot::name;
        msg.sender_id = 1;
        msg.channel = channel;
        msg.content = "!" + command + " could not be found. Type !help to get a list of available commands.";

        writer.send_message(msg);
        user->queue.enqueue(writer);
    }

    return false;
}
