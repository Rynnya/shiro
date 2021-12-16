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

#ifndef SHIRO_STRING_UTILS_HH
#define SHIRO_STRING_UTILS_HH

#include <charconv>
#include <string>

#include "../traits.hh"

namespace shiro::utils::strings {

    template <typename T = int32_t>
    bool evaluate(const std::string& src, T& value) noexcept {
        static_assert(std::is_integral_v<T>, "T should be integral type");

        auto [ptr, ec] = std::from_chars(src.data(), src.data() + src.size(), value);
        return ec == std::errc();
    }

    template <>
    bool evaluate(const std::string& src, float& value) noexcept;

    template <>
    bool evaluate(const std::string& src, double& value) noexcept;

    template <>
    bool evaluate(const std::string& src, bool& value) noexcept;

    template <typename T = int32_t>
    T evaluate(const std::string& src) noexcept {
        static_assert(std::is_integral_v<T>, "T should be integral type");

        T result = traits::basic_initialization<T>::value;
        static_cast<void>(std::from_chars(src.data(), src.data() + src.size(), result));
        return result;
    }

    template <>
    float evaluate(const std::string& src) noexcept;

    template <>
    double evaluate(const std::string& src) noexcept;

    template <>
    bool evaluate(const std::string& src) noexcept;

    std::string to_string(bool src) noexcept;

}

#endif //SHIRO_STRING_UTILS_HH
