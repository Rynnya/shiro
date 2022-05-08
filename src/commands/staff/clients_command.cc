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

#include "../../permissions/permissions.hh"
#include "../../users/user_manager.hh"
#include "../../thirdparty/fmt/format.hh"
#include "../../utils/bot_utils.hh"
#include "../../utils/osu_client.hh"
#include "../../utils/string_utils.hh"
#include "clients_command.hh"

using fmt::format;

bool shiro::commands::clients(std::deque<std::string>& args, const std::shared_ptr<shiro::users::user>& user, const std::string& channel) {
    if (args.size() >= 2) {
        utils::bot::respond("Usage: !clients [user]", user, channel, true);
        return false;
    }

    if (!shiro::users::manager::has_permissions(user, permissions::perms::info_users)) {
        utils::bot::respond("Permission denied. (Client info permission required)", user, channel, true);
        return false;
    }

    if (args.size() == 1) {
        std::shared_ptr<users::user> target = users::manager::get_user_by_username(args.at(0));

        if (target == nullptr) {
            utils::bot::respond(format("{} is currently not online or does not exist.", args.at(0)), user, channel, true);
            return false;
        }

        const std::string client = format(
            "{} is using {}. ({} -> {})",
            target->presence.username,
            utils::clients::to_pretty_string(utils::clients::osu_client::_from_integral(target->client_type)),
            target->client_version,
            target->client_build
        );

        utils::bot::respond(client, user, channel, true);
        return true;
    }

    std::vector<std::string> lines {};

    for (const utils::clients::osu_client &client : utils::clients::osu_client::_values()) {
        std::vector<std::shared_ptr<users::user>> users = utils::clients::get_users(client);

        if (users.empty()) {
            lines.emplace_back(format("{} (0): No one uses this type.", utils::clients::to_pretty_string(client)));
            continue;
        }

        std::string line = format("{} ({}): ", utils::clients::to_pretty_string(client), users.size());

        // We cannot use format("{}", fmt::join here, as shiro::user is custom type :c
        for (const std::shared_ptr<users::user> &client_user : users) {
            line += client_user->presence.username + ", ";
        }

        lines.emplace_back(line.substr(0, line.size() - 2));
    }

    for (const std::string &line : lines) {
        utils::bot::respond(line, user, channel, true);
    }

    return true;
}
