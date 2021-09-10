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
#include <sstream>
#include <utility>

#include "../logger/sentry_logger.hh"
#include "../thirdparty/loguru.hh"
#include "../utils/crypto.hh"
#include "../utils/string_utils.hh"
#include "replay.hh"

std::string shiro::replays::action::to_string() const {
    return std::to_string(this->w) + "|" +
           std::to_string(this->x) + "|" +
           std::to_string(this->y) + "|" +
           std::to_string(this->z) + ",";
}

shiro::replays::replay::replay(shiro::scores::score s, std::string replay)
    : score(std::move(s))
    , raw_replay(std::move(replay)) {
    // Initialized in initializer list
}

void shiro::replays::replay::parse() {
    std::string decompressed = utils::crypto::lzma::decompress(this->raw_replay);

    if (decompressed.empty())
        return;

    std::vector<std::string> parts;
    boost::split(parts, decompressed, boost::is_any_of(","));

    for (const std::string &part : parts) {
        action a;

        std::vector<std::string> pieces;
        boost::split(pieces, part, boost::is_any_of("|"));

        if (pieces.size() < 4)
            continue;

        bool parse_result = true;
        parse_result &= utils::strings::safe_ll(pieces.at(0), a.w);
        parse_result &= utils::strings::safe_float(pieces.at(1), a.x);
        parse_result &= utils::strings::safe_float(pieces.at(2), a.y);
        parse_result &= utils::strings::safe_int(pieces.at(3), a.z);

        if (!parse_result)
        {
            LOG_F(ERROR, "Unable to cast action values into correct data types.");
            logging::sentry::exception(std::invalid_argument("Action string was invalid."));
            continue;
        }

        this->actions.emplace_back(a);
    }
}

std::vector<shiro::replays::action> shiro::replays::replay::get_actions() {
    return this->actions;
}

std::string shiro::replays::replay::to_string() const
{
    static std::stringstream stream;
    stream.clear();

    for (const action &a : this->actions)
        stream << a.to_string();

    return stream.str();
}
