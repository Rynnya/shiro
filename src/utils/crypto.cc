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

#include <cstring>
#include <lzma.h>
#include <memory>
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <ctime>

#include "../thirdparty/cppcrypto/cbc.hh"
#include "../thirdparty/cppcrypto/rijndael.hh"
#include "../thirdparty/websocketpp/base64/base64.hh"
#include "crypto.hh"
#include "../native/process_info.hh"

std::string shiro::utils::crypto::base64::decode(const std::string& base64) {
    return websocketpp::base64_decode(base64);
}

std::string shiro::utils::crypto::rijndael256::decode(const std::string& iv, const std::string& key, const std::string& cipher) {
    using namespace cppcrypto;

    std::unique_ptr<cbc> decryptor = std::make_unique<cbc>(rijndael256_256());
    decryptor->init(reinterpret_cast<const unsigned char*>(key.data()), key.size(), reinterpret_cast<const unsigned char*>(&iv[0]), iv.size(), block_cipher::direction::decryption);

    std::vector<unsigned char> result;
    decryptor->decrypt_update(reinterpret_cast<const unsigned char*>(&cipher[0]), cipher.size(), result);
    decryptor->decrypt_final(result);

    return std::string(reinterpret_cast<char*>(result.data()), result.size());
}

std::string shiro::utils::crypto::pbkdf2_hmac_sha512::hash(const std::string& input, const std::string& salt) {
    unsigned char result[64];
    char output[256];

    PKCS5_PBKDF2_HMAC(input.c_str(), input.length(), reinterpret_cast<const unsigned char*>(salt.c_str()), salt.size(), 4096, EVP_sha512(), 64, result);

    for (size_t i = 0; i < 64; i++) {
        std::snprintf(&output[i * 2], sizeof(output), "%02x", (unsigned int) result[i]);
    }

    return std::string(output);
}

std::string shiro::utils::crypto::md5::hash(const std::string &input) {
    unsigned char result[16];
    char output[33];

    MD5((unsigned char*) input.c_str(), input.length(), result);

    for (size_t i = 0; i < 16; i++) {
        std::snprintf(&output[i * 2], sizeof(output), "%02x", (unsigned int) result[i]);
    }

    return std::string(output);
}

std::string shiro::utils::crypto::lzma::decompress(const std::string& input) {
    lzma_stream stream = LZMA_STREAM_INIT;

    lzma_ret decoder = lzma_alone_decoder(&stream, UINT64_MAX);

    if (decoder != LZMA_OK) {
        return "";
    }

    std::string output;
    output.resize(BUFSIZ);

    size_t amount = 0;
    size_t available = output.size();

    stream.next_in = reinterpret_cast<const uint8_t*>(input.data());
    stream.avail_in = input.size() * 4; // Dirty hack to make LZMA read whole file instead of compressed size * 2
    stream.next_out = reinterpret_cast<uint8_t*>(&output[0]);
    stream.avail_out = available;

    while (true) {
        decoder = lzma_code(&stream, stream.avail_in == 0 ? LZMA_FINISH : LZMA_RUN);
        if (decoder == LZMA_STREAM_END) {
            amount += available - stream.avail_in;

            if (stream.avail_in == 0) {
                break;
            }

            output.resize(amount);
            lzma_end(&stream);
            return output;
        }

        if (decoder != LZMA_OK) {
            break;
        }

        if (stream.avail_out == 0) {
            amount += available - stream.avail_out;
            output.resize(input.size() * 4 << 1);
            stream.next_out = reinterpret_cast<uint8_t*>(&output[0] + amount);
            stream.avail_out = available = output.size() - amount;
        }
    }

    return output;
}

uint32_t shiro::utils::crypto::get_highest_bit(uint32_t bitwise) {
    if (bitwise == 0) {
        return 0;
    }

    uint32_t result = 1;

    while (bitwise >>= 1) {
        result <<= 1;
    }

    return result;
}

uint32_t shiro::utils::crypto::make_seed() {
    int32_t a = std::clock();
    int32_t b = std::time(nullptr);
    int32_t c = native::process_info::get_pid();

    a -= b;
    a -= c;
    a ^= c >> 13;

    b -= c;
    b -= a;
    b ^= a << 8;

    c -= a;
    c -= b;
    c ^= b >> 13;

    a -= b;
    a -= c;
    a ^= c >> 12;

    b -= c;
    b -= a;
    b ^= a << 16;

    c -= a;
    c -= b;
    c ^= b >> 5;

    a -= b;
    a -= c;
    a ^= c >> 3;

    b -= c;
    b -= a;
    b -= a << 10;

    c -= a;
    c -= b;
    c ^= b >> 15;

    return static_cast<uint32_t>(c);
}
