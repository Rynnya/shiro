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

#ifndef SHIRO_TRAITS_HH
#define SHIRO_TRAITS_HH

#include <string>
#include <vector>

namespace shiro::traits {

    namespace detail {

        template <typename T>
        struct is_vector : std::false_type {};

        template <typename T>
        struct is_vector<std::vector<T>> : std::true_type {};

        template <typename T>
        struct is_string : std::false_type {};

        template <>
        struct is_string<std::string> : std::true_type {};

    }

    template <typename T, typename E = void>
    struct basic_initialization {};

    template <typename T>
    struct basic_initialization<T, std::enable_if_t<std::is_unsigned_v<T>>> {
        static constexpr T value = 0U;
    };

    template <typename T>
    struct basic_initialization<T, std::enable_if_t<std::is_signed_v<T>>> {
        static constexpr T value = -1;
    };

    template <typename T>
    struct basic_initialization<T, std::enable_if_t<detail::is_vector<T>::value>> {
        static constexpr T value = {};
    };

    template <typename T>
    struct basic_initialization<T, std::enable_if_t<detail::is_string<T>::value>> {
        static constexpr T value = "";
    };
}

#endif