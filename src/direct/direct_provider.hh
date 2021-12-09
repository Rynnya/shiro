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

#ifndef SHIRO_DIRECT_PROVIDER_HH
#define SHIRO_DIRECT_PROVIDER_HH

#include <string>
#include <tuple>
#include <memory>
#include <unordered_map>

#include "../thirdparty/crow.hh"

namespace shiro::direct {

    class direct_provider {
    public:
        virtual ~direct_provider() = default;

        virtual void search(crow::response&& callback, std::unordered_map<std::string, std::string> parameters) = 0;
        virtual void search_np(crow::response&& callback, std::unordered_map<std::string, std::string> parameters) = 0;
        virtual void download(crow::response&& callback, int32_t beatmap_id, bool no_video) = 0;

        virtual const std::string name() const = 0;
        uint32_t hold_callback(crow::response&& callback);

        // Counter, required to properly insert values inside cache system
        // Not used in Hanaru, as this provider works on own cache system
        std::atomic_uint32_t counter = 0;
        std::unordered_map<uint32_t, crow::response> holder = {};
    };

    extern std::shared_ptr<direct_provider> provider;

    void init();
}

#endif //SHIRO_DIRECT_PROVIDER_HH
