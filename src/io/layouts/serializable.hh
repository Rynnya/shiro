/*
 * shiro - High performance, high quality osu!Bancho C++ re-implementation
 * Copyright (C) 2018-2020 Marc3842h, czapek
 * Copyright (C) 2021 Rynnya
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

#ifndef SHIRO_SERIALIZABLE_HH
#define SHIRO_SERIALIZABLE_HH

#include "../osu_buffer.hh"
#include "../../traits.hh"

namespace shiro::io {

    template <typename T = uint8_t>
    class serializable {
    public:
        typedef T data_type;

        T data = traits::basic_initialization<T>::value;

        serializable() = default;
        explicit serializable(T data) : data(data) {}

        virtual shiro::io::buffer marshal() {
            buffer buf;
            buf.write<data_type>(data);
            return buf;
        }

        virtual void unmarshal(buffer& buf) { /* Nothing, derived classes will override it */ }

        virtual int32_t get_size() {
            return static_cast<int32_t>(this->marshal().get_size());
        }
    };

    // Empty serializable class for derived classes, without additional containers
    template <>
    class serializable<void> {
    public:
        virtual shiro::io::buffer marshal();
        virtual void unmarshal(buffer& buf);

        virtual int32_t get_size();
    };

    template <>
    serializable<std::string>::serializable(std::string data);

    template <>
    serializable<std::vector<int32_t>>::serializable(std::vector<int32_t> data);

    template <>
    shiro::io::buffer serializable<std::string>::marshal();
}

#endif //SHIRO_SERIALIZABLE_HH
