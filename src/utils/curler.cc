﻿/*
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

#include <curl/curl.h>

#include "../config/direct_file.hh"
#include "../logger/sentry_logger.hh"
#include "../thirdparty/naga.hh"
#include "curler.hh"

std::tuple<bool, std::string> shiro::utils::curl::get(const std::string &url) {
    CURL *curl = curl_easy_init();
    CURLcode status_code;

    if (curl == nullptr) {
        return { false, "Unable to acquire curl handle." };
    }

    std::string output;

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, internal_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &output);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "shiro (https://github.com/Rynnya/shiro)");
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    status_code = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (status_code == CURLE_OK) {
        logging::sentry::http_request_out(url, "GET", status_code, url.find("/osu/") == std::string::npos && output.size() < 100000 ? output : "");
        return { true, output };
    }

    output = curl_easy_strerror(status_code);

    logging::sentry::http_request_out(url, "GET", status_code, output);
    return { false, output };
}

std::tuple<bool, std::string> shiro::utils::curl::get_direct(const std::string &url) {
    CURL *curl = curl_easy_init();
    CURLcode status_code;

    if (curl == nullptr) {
        return { false, "Unable to acquire curl handle." };
    }

    std::string output;

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, internal_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &output);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    switch (config::direct::provider) {
        case 0: {
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "shiro (https://github.com/Rynnya/shiro)");
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            break;
        }
        case 1: {
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "osu!");

            // osu! mirrors use self signed certificates that don't pass SSL peer certificate check
            // Disable the peer and verification checks for this one request
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);

            break;
        }
        case 2: {
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "shiro (https://github.com/Rynnya/shiro)");

            const static std::string header = fmt::format("Token: {}", config::direct::api_key);
            struct curl_slist *chunk = nullptr;

            chunk = curl_slist_append(chunk, "cho-server: shiro (https://github.com/Rynnya/shiro)");
            chunk = curl_slist_append(chunk, "Content-Type: application/json");
            chunk = curl_slist_append(chunk, header.c_str());

            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
            break;
        }
        case 3: {
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "shiro (https://github.com/Rynnya/shiro)");
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            break;
        }
        default: {
            // All valid cases are covered above
            break;
        }
    }

    status_code = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (status_code == CURLE_OK) {
        logging::sentry::http_request_out(url, "GET", status_code, url.find("/osu/") == std::string::npos && output.size() < 100000 ? output : "");
        return { true, output };
    }

    output = curl_easy_strerror(status_code);

    logging::sentry::http_request_out(url, "GET", status_code, output);
    return { false, output };
}

bool shiro::utils::curl::post_message(const std::string& url, const nlohmann::json& message) {
    CURL* curl = curl_easy_init();
    CURLcode status_code;

    if (curl == nullptr) {
        return false;
    }

    std::string output;

    curl_easy_setopt(curl, CURLOPT_USERAGENT, "shiro (https://github.com/Rynnya/shiro)");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, internal_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &output);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    std::string msg = message.dump();
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, msg.c_str());

    struct curl_slist* chunk = nullptr;
    chunk = curl_slist_append(chunk, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

    status_code = curl_easy_perform(curl);

    long http_status_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_status_code);

    curl_easy_cleanup(curl);

    if (status_code == CURLE_OK) {
        return true;
    }

    // Replace original url with fake to avoid leaks
    logging::sentry::http_request_out("https://discord.com/api/webhooks/id/token", "POST", http_status_code, output);
    return false;
}

std::string shiro::utils::curl::escape_url(const std::string &raw) {
    CURL *curl = curl_easy_init();
    char *curl_result = curl_easy_escape(curl, raw.c_str(), 0);
    std::string result = curl_result;

    curl_free(curl_result);
    curl_easy_cleanup(curl);
    return result;
}

std::string shiro::utils::curl::unescape_url(const std::string &raw) {
    CURL *curl = curl_easy_init();
    char *curl_result = curl_easy_unescape(curl, raw.c_str(), 0, nullptr);
    std::string result = curl_result;

    curl_free(curl_result);
    curl_easy_cleanup(curl);
    return result;
}

size_t shiro::utils::curl::internal_callback(void *raw_data, size_t size, size_t memory, std::string *ptr) {
    size_t new_length = size * memory;
    size_t old_length = ptr->size();

    try {
        ptr->resize(old_length + new_length);
    } catch (const std::bad_alloc &ex) { // do you even think this might happend? if so - then whole shiro will crash ¯\_(ツ)_/¯
        LOG_F(ERROR, "Unable to allocate new memory for http response: {}.", ex.what());
        CAPTURE_EXCEPTION(ex);

        return 0;
    }

    std::copy(static_cast<char*>(raw_data), static_cast<char*>(raw_data) + new_length, ptr->begin() + old_length);
    return size * memory;
}
