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

#ifndef SHIRO_SERIALIZABLE_HH
#define SHIRO_SERIALIZABLE_HH

#include "../osu_buffer.hh"
#include "../../traits.hh"

namespace shiro::io {

    template <typename T = uint8_t>
    class serializable {
    public:
        static_assert(
            std::is_arithmetic<T>::value || traits::detail::is_string<T>::value || traits::detail::is_vector<T>::value,
            "serializable type must be arithmetic, std::string or std::vector (void specialization declared below)"
        );

        typedef T data_type;

        data_type data = traits::basic_initialization<T>::value;

        serializable() noexcept = default;
        explicit serializable(data_type data) noexcept : data(data) {}

        virtual ~serializable() = default;

        serializable(const serializable& other) = delete;
        serializable(serializable&& other) = delete;

        serializable& operator=(const serializable& other) = delete;
        serializable& operator=(serializable&& other) = delete;

        virtual shiro::io::buffer marshal() {
            buffer buf;
            buf.write<data_type>(data);
            return buf;
        }

        virtual void unmarshal(buffer& buf) noexcept { /* Nothing, derived classes will override it */ }

        virtual int32_t get_size() {
            return static_cast<int32_t>(this->marshal().get_size());
        }
    };

    // Empty serializable class for derived classes, without additional containers
    template <>
    class serializable<void> {
    public:
        virtual shiro::io::buffer marshal() = 0;
        virtual void unmarshal(buffer& buf) = 0;

        virtual int32_t get_size() = 0;
    };

    serializable<std::string>::serializable(std::string data) noexcept;
    shiro::io::buffer serializable<std::string>::marshal();

    serializable<std::vector<int32_t>>::serializable(std::vector<int32_t> data) noexcept;
}

#endif //SHIRO_SERIALIZABLE_HH
