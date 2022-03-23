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

#ifndef SHIRO_EMULATION_HH
#define SHIRO_EMULATION_HH

#include "../direct_provider.hh"

namespace shiro::direct {

    class emulation : public direct_provider {
    public:
        void search(crow::response& callback, std::unordered_map<std::string, std::string> parameters) override;
        void search_np(crow::response& callback, std::unordered_map<std::string, std::string> parameters) override;
        void download(crow::response& callback, int32_t beatmap_id, bool no_video) override;

        const std::string name() const override;

    };

}

#endif //SHIRO_EMULATION_HH
