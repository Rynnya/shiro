/*
 * shiro - High performance, high quality osu!Bancho C++ re-implementation
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

#ifndef SHIRO_HANARU_HH
#define SHIRO_HANARU_HH

#include <mutex>
#include <unordered_set>

#include "../direct_provider.hh"
#include "../../thirdparty/websocketpp/client.hh"
#include "../../thirdparty/websocketpp/config/asio_no_tls_client.hh"

namespace shiro::direct {

    typedef websocketpp::client<websocketpp::config::asio_client> client;

    class hanaru : public direct_provider {
    public:
        hanaru();

        void search(crow::response& callback, std::unordered_map<std::string, std::string> parameters) override;
        void search_np(crow::response& callback, std::unordered_map<std::string, std::string> parameters) override;
        void download(crow::response& callback, int32_t beatmap_id, bool no_video) override;

        const std::string name() const override;

    private:
        void on_message(websocketpp::connection_hdl handle, client::message_ptr msg);

        int32_t sanitize_mode(const std::string& value);
        int32_t sanitize_status(const std::string& value);

        client socket;
        client::connection_ptr connection_ptr;

        std::unordered_map<int32_t, std::vector<std::function<void(int32_t, const std::string&)>>> cache = {};

        std::stringstream socket_stream;
        std::mutex mtx;
    };

}

#endif //SHIRO_HANARU_HH