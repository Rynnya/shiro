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

#include "serializable.hh"
#include "../../utils/osu_string.hh"

template<typename T>
shiro::io::serializable<T>::serializable(T data) : data(data) {}
shiro::io::serializable<std::string>::serializable(std::string data) : data(std::move(data)) {}
shiro::io::serializable<std::vector<int32_t>>::serializable(std::vector<int32_t> data) : data(std::move(data)) {}

template <typename T>
shiro::io::buffer shiro::io::serializable<T>::marshal() {
    buffer buf;
    buf.write<data_type>(data);
    return buf;
}

shiro::io::buffer shiro::io::serializable<std::string>::marshal() {
    buffer buf;
    buf.write<std::string>(utils::osu_string(data));
    return buf;
}

template <typename T>
void shiro::io::serializable<T>::unmarshal(buffer& buf) {
    // Nothing, every derived class will do what they need here
}

template <typename T>
int32_t shiro::io::serializable<T>::get_size() {
    return static_cast<int32_t>(this->marshal().get_size());
}

// Specification for empty serializable class

shiro::io::buffer shiro::io::serializable<void>::marshal() {
    return buffer();
};

void shiro::io::serializable<void>::unmarshal(buffer& buf) {};

int32_t shiro::io::serializable<void>::get_size() {
    return 0;
}