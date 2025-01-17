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

#include <functional>
#include <unordered_map>
#include <utility>
#include <memory>

#include "../commands/multiplayer/abort_command.hh"
#include "../commands/multiplayer/help_command.hh"
#include "../commands/multiplayer/host_command.hh"
#include "../commands/multiplayer/invite_command.hh"
#include "../commands/multiplayer/lock_command.hh"
#include "../commands/multiplayer/map_command.hh"
#include "../commands/multiplayer/password_command.hh"
#include "../commands/multiplayer/size_command.hh"
#include "../commands/multiplayer/unlock_command.hh"

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
#include "../thirdparty/naga.hh"
#include "../thirdparty/uuid.hh"
#include "../users/user_manager.hh"
#include "../utils/escaper.hh"
#include "../utils/osu_client.hh"
#include "../utils/time_utils.hh"
#include "../utils/string_utils.hh"
#include "bot.hh"

std::shared_ptr<shiro::users::user> shiro::bot::bot_user = nullptr;
static std::unordered_map<std::string, std::function<bool(std::deque<std::string>&, std::shared_ptr<shiro::users::user>, std::string)>> commands_map;
static std::unordered_map<std::string, std::function<bool(std::deque<std::string>&, std::shared_ptr<shiro::users::user>, std::string)>> commands_mp_map;

void shiro::bot::init() {
    auto db = shiro::database::instance->pop();

    {
        auto result = db(select(tables::users_table.id).from(tables::users_table).where(tables::users_table.id == 1).limit(1u));

        // Check if the bot user exists, if not insert it into the db
        if (result.empty()) {
            db(insert_into(tables::users_table).set(
                tables::users_table.id = 1,
                tables::users_table.username = config::bot::name,
                tables::users_table.safe_username = utils::escaper::make_safe(config::bot::name),
                tables::users_table.password_hash = digestpp::sha256().absorb(config::database::password).hexdigest(),
                tables::users_table.salt = config::database::database,
                tables::users_table.email = config::bot::name + "@shiro.host",
                tables::users_table.ip = "127.0.0.1",
                tables::users_table.registration_date = 0,
                tables::users_table.latest_activity = 0,
                tables::users_table.followers = 0,
                tables::users_table.permissions = 0xDEADCAFE, // Special role for robots
                tables::users_table.userpage = "beep boop",
                tables::users_table.country = "JP"
            ));
        }
    }

    // Creates user stats in Classic table
    {
        auto result = db(select(tables::users_stats_table.id).from(tables::users_stats_table).where(tables::users_stats_table.id == 1).limit(1u));

        if (result.empty()) {
            db(insert_into(tables::users_stats_table).set(tables::users_stats_table.id = 1));
        }
    }

    // Creates user stats in Relax table
    {
        auto result = db(select(tables::users_relax_table.id).from(tables::users_relax_table).where(tables::users_relax_table.id == 1).limit(1u));

        if (result.empty()) {
            db(insert_into(tables::users_relax_table).set(tables::users_relax_table.id = 1));
        }
    }

    std::shared_ptr<users::user> bot_user = std::make_shared<users::user>(1);

    if (!bot_user->init()) {
        ABORT_F("Unable to initialize chat bot.");
    }

    bot_user->token = sole::uuid4().str();
    bot_user->hwid = digestpp::sha256().absorb(config::bot::name).hexdigest();
    bot_user->last_ping = utils::time::current_time();

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

    LOG_F(INFO, "Bot has been successfully registered as {} and is now online.", config::bot::name);
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

    LOG_F(INFO, "Bot commands have been successfully loaded. {} commands available.", commands_map.size());

    commands_mp_map.insert(std::make_pair("mp", commands_mp::help));
    commands_mp_map.insert(std::make_pair("help", commands_mp::help));

    commands_mp_map.insert(std::make_pair("abort", commands_mp::abort));
    commands_mp_map.insert(std::make_pair("host", commands_mp::host));
    commands_mp_map.insert(std::make_pair("invite", commands_mp::invite));
    commands_mp_map.insert(std::make_pair("lock", commands_mp::lock));
    commands_mp_map.insert(std::make_pair("unlock", commands_mp::unlock));
    commands_mp_map.insert(std::make_pair("map", commands_mp::map));
    commands_mp_map.insert(std::make_pair("password", commands_mp::password));
    commands_mp_map.insert(std::make_pair("size", commands_mp::size));

    LOG_F(INFO, "Multiplayer commands have been successfully loaded. {} commands available.", commands_mp_map.size());
}

bool shiro::bot::handle(const std::string &command, std::deque<std::string> &args, std::shared_ptr<shiro::users::user> user, std::string channel) {
    auto cmd = commands_map.find(command);
    if (cmd != commands_map.end()) {
        return cmd->second(args, user, channel);
    }

    io::layouts::message msg;
    io::osu_writer writer;

    msg.sender = config::bot::name;
    msg.sender_id = 1;
    msg.channel = channel;
    msg.content = fmt::format("!{} could not be found. Type !help to get a list of available commands.", command);

    writer.send_message(msg);
    user->queue.enqueue(writer);

    return false;
}

bool shiro::bot::handle_mp(const std::string& command, std::deque<std::string>& args, std::shared_ptr<shiro::users::user> user, std::string channel) {
    auto cmd = commands_mp_map.find(command);
    if (cmd != commands_mp_map.end()) {
        return cmd->second(args, user, channel);
    }

    io::layouts::message msg;
    io::osu_writer writer;

    msg.sender = config::bot::name;
    msg.sender_id = 1;
    msg.channel = channel;
    msg.content = fmt::format("!mp {} could not be found. Type !mp help to get a list of available commands.", command);

    writer.send_message(msg);
    user->queue.enqueue(writer);

    return false;
}