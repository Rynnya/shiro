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

#ifndef SHIRO_OSU_BUFFER_HH
#define SHIRO_OSU_BUFFER_HH

#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

namespace shiro::io {

    class buffer {
    private:
        std::vector<uint8_t> bytes;

        size_t written_size = 0;
        size_t position = 0;

        void allocate(size_t amount);

    public:
        buffer() = default;
        buffer(const buffer &buf);
        explicit buffer(std::string data);

        void append(std::string data);
        void append(buffer &buf);

        template <typename T = uint8_t>
        void write(T data) {
            this->allocate(sizeof(T));
            uint8_t *data_arr = reinterpret_cast<uint8_t*>(&data);

            for (size_t i = 0; i < sizeof(T); i++) {
                this->bytes.at(this->written_size++) = data_arr[i];
            }
        }

        template <typename T = uint8_t>
        T read() {
            T data = *reinterpret_cast<T*>((uintptr_t)this->bytes.data() + this->position);
            this->position += sizeof(T);

            return data;
        }

        std::string serialize();

        bool can_read(size_t amount) noexcept;
        bool is_empty() noexcept;

        void clear() noexcept;
        void seek(size_t position) noexcept;
        void advance(size_t amount) noexcept;

        size_t get_size() noexcept;

    };

    template <>
    void buffer::write(std::string data);

    template <>
    void buffer::write(std::vector<int32_t> data);

    template <>
    std::string buffer::read();

    template <>
    std::vector<int32_t> buffer::read();

}


#endif //SHIRO_OSU_BUFFER_HH
