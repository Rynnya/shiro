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

#include <boost/algorithm/string.hpp>

#include "../users/user_manager.hh"
#include "osu_client.hh"

shiro::utils::clients::osu_client shiro::utils::clients::parse_version(const std::string &client_version, const int32_t &client_build) {
    // 3rd party osu! clients
    if (client_build == 20161205 && client_version.find("cuttingedge"))
        return osu_client::osu_fx;

    if (client_build == 20181018 && client_version.find("noxna") != std::string::npos)
        return osu_client::banana_client;

    if (client_version.find("banana") != std::string::npos)
        return osu_client::banana_client;

    if (client_version.find("version") != std::string::npos)
        return osu_client::tsuki;

    if (boost::algorithm::to_lower_copy(client_version).find("yozora") != std::string::npos)
        return osu_client::yozora;

    // osu!fallback and very outdated versions
    if (client_build <= 20160403)
        return osu_client::fallback;

    std::string::size_type first_dot = client_version.find('.');

    // osu!lazer contains two dots in their version number, unlike stable clients
    if (first_dot != std::string::npos && client_version.find('.', first_dot + 1) != std::string::npos)
        return osu_client::lazer;

    // Official osu! clients available for download on the osu! website
    if (client_version.find("beta") != std::string::npos)
        return osu_client::beta;

    if (client_version.find("cuttingedge") != std::string::npos)
        return osu_client::cutting_edge;

    if (client_version.find("tourney") != std::string::npos)
        return osu_client::tournament;

    // Suspicious clients as these can't be obtained legit from the osu! website
    if (client_version.find("dev") != std::string::npos)
        return osu_client::dev;

    if (client_version.find("public_test") != std::string::npos)
        return osu_client::public_test;

    if (client_version.find("noxna") != std::string::npos)
        return osu_client::no_xna;

    // osu!Stable
    std::string subversion = client_version.substr(1);

    if (subversion.find('.') != std::string::npos)
        subversion.erase(subversion.find('.'));

    subversion.erase(std::remove_if(subversion.begin(), subversion.end(), [](char c) {
        return std::isdigit(c);
    }), subversion.end());

    if (subversion.empty())
        return osu_client::stable;

    LOG_F(INFO, "Tried to process osu! version with unknown version: %s (%i -> %s)", client_version.c_str(), client_build, subversion.c_str());

    return osu_client::unknown;
}

bool shiro::utils::clients::is_official(const shiro::utils::clients::osu_client &client) {
    return client > +osu_client::unknown && client < +osu_client::aschente;
}

bool shiro::utils::clients::is_thirdparty(const shiro::utils::clients::osu_client &client) {
    return client > +osu_client::aschente;
}

bool shiro::utils::clients::is_suspicious(const shiro::utils::clients::osu_client &client) {
    return client == +osu_client::unknown ||
           client == +osu_client::dev ||
           client == +osu_client::public_test ||
           client == +osu_client::no_xna;
}

std::vector<std::shared_ptr<shiro::users::user>> shiro::utils::clients::get_users(const shiro::utils::clients::osu_client &client) {
    std::vector<std::shared_ptr<users::user>> result;

    users::manager::iterate([client, &result](std::shared_ptr<users::user> user) {
        if (user->client_type != client)
            return;

        result.emplace_back(user);
    });

    return result;
}

std::string shiro::utils::clients::to_pretty_string(const shiro::utils::clients::osu_client &client) {
    std::string result = client._to_string();

    std::replace(result.begin(), result.end(), '_', ' ');
    result.at(0) = std::toupper(result.at(0));

    std::string::size_type index = result.find(' ');

    if (index != std::string::npos)
        result.at(index + 1) = std::toupper(result.at(index + 1));

    return result;
}
