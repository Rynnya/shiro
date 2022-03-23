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

#ifndef SHIRO_CRYPTO_HH
#define SHIRO_CRYPTO_HH

#include <string>
#include <vector>

namespace shiro::utils::crypto {

    namespace base64 {
        std::string decode(const std::string& base64);
    }

    namespace rijndael256 {
        std::string decode(const std::string& iv, const std::string& key, const std::string& cipher);
    }

    namespace pbkdf2_hmac_sha512 {
        std::string hash(const std::string& input, const std::string& salt);
    }

    namespace md5 {
        std::string hash(const std::string &input);
    }

    namespace lzma {
        std::string decompress(const std::string& input);
    }

    uint32_t get_highest_bit(uint32_t bitwise);

    uint32_t make_seed();

}

#endif //SHIRO_CRYPTO_HH
