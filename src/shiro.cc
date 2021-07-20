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

#include <cstdlib>
#include <curl/curl.h>
#include <thread>

#include "beatmaps/beatmap_helper.hh"
#include "bot/bot.hh"
#include "channels/channel_manager.hh"
#include "channels/console_osu_bridge.hh"
#include "channels/discord_webhook.hh"
#include "config/api_file.hh"
#include "config/bancho_file.hh"
#include "config/bot_file.hh"
#include "config/cli_args.hh"
#include "config/db_file.hh"
#include "config/direct_file.hh"
#include "config/discord_webhook_file.hh"
#include "config/ipc_file.hh"
#include "config/score_submission_file.hh"
#include "direct/direct_provider.hh"
#include "geoloc/geoloc.hh"
#include "geoloc/maxmind_resolver.hh"
#include "logger/logger.hh"
#include "logger/sentry_logger.hh"
#include "native/process_info.hh"
#include "native/signal_handler.hh"
#include "native/system_statistics.hh"
#include "permissions/role_manager.hh"
#include "replays/replay_manager.hh"
#include "routes/routes.hh"
#include "thirdparty/cli11.hh"
#include "thirdparty/loguru.hh"
#include "users/user_activity.hh"
#include "users/user_punishments.hh"
#include "users/user_timeout.hh"
#include "utils/crypto.hh"
#include "utils/curler.hh"
#include "shiro.hh"

std::shared_ptr<shiro::database> shiro::db_connection = nullptr;
std::shared_ptr<shiro::redis> shiro::redis_connection = nullptr;
tsc::TaskScheduler shiro::scheduler;
std::time_t shiro::start_time = std::time(nullptr);
std::string shiro::commit = "42704f8";

int shiro::init(int argc, char **argv) {
    logging::init(argc, argv);

    curl_global_init(CURL_GLOBAL_DEFAULT);

    std::srand(utils::crypto::make_seed());

    config::cli::parse(argc, argv);

    // Parse Bancho config before every other config to check if integrations are enabled
    config::bancho::parse();

    // Launch Sentry.io and DataDog integrations
    logging::sentry::init();

    config::api::parse();
    config::bot::parse();
    config::database::parse();
    config::direct::parse();
    config::ipc::parse();
    config::discord_webhook::parse();
    config::score_submission::parse();

    beatmaps::helper::init();
    direct::init();
    geoloc::init();
    channels::discord_webhook::init();

    db_connection = std::make_shared<database>(
            config::database::address, config::database::port, config::database::database,
            config::database::username, config::database::password
    );
    db_connection->connect();
    db_connection->setup();

    redis_connection = std::make_shared<redis>(
            config::database::redis_address,
            config::database::redis_port,
            config::database::redis_password
    );
    redis_connection->connect();
    redis_connection->setup();

    std::thread scheduler_updater([]() {
        while (true) {
            std::this_thread::sleep_for(1ms);
            scheduler.Update(1ms);
        }
    });
    scheduler_updater.detach(); // The root of all suffering is attachment

    roles::manager::init();

    bot::init();
    bot::init_commands();

    channels::manager::init();

    users::activity::init();
    users::punishments::init();
    users::timeout::init();

    channels::bridge::install();

    replays::init();

    native::system_stats::init();
    native::signal_handler::install();

    LOG_F(INFO, "Welcome to Shiro. Listening on http://%s:%i/.", config::bancho::host.c_str(), config::bancho::port);
    LOG_F(INFO, "Press CTRL + C to quit.");

    routes::init();

    return EXIT_SUCCESS;
}

void shiro::destroy() {
    redis_connection->disconnect();

    curl_global_cleanup();

    geoloc::maxmind::destroy();

    scheduler.CancelAll();

    using namespace shiro::channels;
    if (shiro::config::discord_webhook::enabled)
    {
        nlohmann::json message = discord_webhook::create_basis();
        message["embeds"].push_back(discord_webhook::create_embed("Shiro is shutting down... Bye!", "", (uint32_t)discord_webhook::colors::Blurple));
        shiro::utils::curl::post_message(shiro::config::discord_webhook::url, message);
    }

    LOG_F(INFO, "Thank you and goodbye.");
}
