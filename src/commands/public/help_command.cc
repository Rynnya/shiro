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

#include "../../users/user_manager.hh"
#include "../../utils/bot_utils.hh"
#include "help_command.hh"

const std::unordered_map<shiro::permissions::perms, std::string> shiro::commands::staff_commands = {
    { shiro::permissions::perms::announce_cmd,      "!announce - Sends a announcement to everyone or a specific user"   },
    { shiro::permissions::perms::ban_users,         "!ban - Bans a player"                                              },
    { shiro::permissions::perms::clear_cmd,         "!clear - Clears the chat for everyone or a specific user"          },
    { shiro::permissions::perms::info_users,        "!clients - Lists game versions of connected players"               },
    { shiro::permissions::perms::kick_users,        "!kick - Kicks a player from the server"                            },
    { shiro::permissions::perms::recalculate_cmd,   "!recalculate - Starts pp recalculation service"                    },
    { shiro::permissions::perms::restart_cmd,       "!restart - Restarts the server"                                    },
    { shiro::permissions::perms::restrict_users,    "!restrict - Restricts a player"                                    },
    { shiro::permissions::perms::rtx_cmd,           "!rtx - Send a rtx to a specific user"                              },
    { shiro::permissions::perms::silence_users,     "!silence - Mutes a player"                                         },
};

bool shiro::commands::help(std::deque<std::string>& args, const std::shared_ptr<shiro::users::user>& user, const std::string& channel) {
    utils::bot::respond("Commands:", user, channel, true);
    utils::bot::respond("!classic - Switch current mode to Classic leaderboard and presence", user, channel, true);
    utils::bot::respond("!help - Shows list of commands", user, channel, true);
    utils::bot::respond("!localclear - Clears your local chat", user, channel, true);
    utils::bot::respond("!relax - Switch current mode to Relax leaderboard and presence", user, channel, true);
    utils::bot::respond("!roll - Rolls a random number", user, channel, true);

    for (auto [permission, response] : staff_commands) {
        if (shiro::users::manager::has_permissions(user, permission)) {
            utils::bot::respond(response, user, channel, true);
        }
    }

    return true;
}
