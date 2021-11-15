#include "crow.hh"

/*
 _____ _____ _____ _ _ _
|     | __  |     | | | |  Crow - a Sentry client for C++
|   --|    -|  |  | | | |  version 0.0.6+
|_____|__|__|_____|_____|  https://github.com/nlohmann/crow

Modified by Rynnya, to make sure that new version of Sentry will work with this code.
List of changes:
 - Upgraded client version from 5 to 7
 - Allowed to recreate m_payload, which don't create memory leaks anymore
 - Changed from asserts and exceptions to exception-free standard (as Sentry requests from clients)

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2018 Niels Lohmann <http://nlohmann.me>.

Permission is hereby  granted, free of charge, to any  person obtaining a copy
of this software and associated  documentation files (the "Software"), to deal
in the Software  without restriction, including without  limitation the rights
to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/*!
 * @file crow.cpp
 * @brief implementation of class crow
 */

#include <exception> // current_exception, exception, get_terminate, rethrow_exception, set_terminate
#include <regex> // regex, regex_match, smatch
#include <stdexcept> // invalid_argument
#include <sstream> // stringstream
#include <iostream>
#include "crow.hh"
#include "crow_config.hh"
#include "crow_utilities.hh"
#include "curl_wrapper.hh"
#include "../json.hh"
#include "../../config/bancho_file.hh"
#include "../../native/system_info.hh"
#include "../../native/system_statistics.hh"
#include "../../thirdparty/loguru.hh"
#include "../../thread/thread_pool.hh"

using json = nlohmann::json;

namespace nlohmann {

    class crow;
}

nlohmann::crow* nlohmann::crow::m_client_that_installed_termination_handler = nullptr;

nlohmann::crow::crow(const std::string& dsn, const json& context, const double sample_rate, const bool install_handlers)
    : m_sample_rate(static_cast<int>(sample_rate * 100.0))
    , m_enabled(not dsn.empty()) {
        // process DSN
        if (not dsn.empty()) {
            std::regex dsn_regex("(http[s]?)://([^:]+)@([^/]+)/([0-9]+)");
            std::regex dsn_compatability_regex("(http[s]?)://([^:]+):([^@]+)@([^/]+)/([0-9]+)");

            std::smatch pieces_match;

            if (std::regex_match(dsn, pieces_match, dsn_regex) and pieces_match.size() == 5) {
                const auto scheme = pieces_match.str(1);
                m_public_key = pieces_match.str(2);
                m_secret_key = "";
                const auto host = pieces_match.str(3);
                const auto project_id = pieces_match.str(4);
                m_store_url = scheme + "://" + host + "/api/" + project_id + "/store/";
            }
            else if (std::regex_match(dsn, pieces_match, dsn_compatability_regex) and pieces_match.size() == 6) {
                dsn_regex = dsn_compatability_regex;
                const auto scheme = pieces_match.str(1);
                m_public_key = pieces_match.str(2);
                m_secret_key = pieces_match.str(3);
                const auto host = pieces_match.str(4);
                const auto project_id = pieces_match.str(5);
                m_store_url = scheme + "://" + host + "/api/" + project_id + "/store/";
            }
            else {
                // We cannot use macro here, because of strange instability in 'operator>', caused by json.hh
                // Also this log really important, even when owner disables warnings
                loguru::log(loguru::Verbosity_WARNING, __FILE__, __LINE__, "DSN Sentry link was invalid, disabling to minimize performance losses.");
                shiro::config::bancho::sentry_integration = false;
                m_enabled = false;
                return;
            }
        }
        else {
            loguru::log(loguru::Verbosity_INFO, __FILE__, __LINE__, "DSN Sentry link was empty, disabling to minimize performance losses.");
            shiro::config::bancho::sentry_integration = false;
            m_enabled = false;
            return;
        }

        // manage context
        clear_context();
        merge_context(context);

        // install termination handler
        if (install_handlers) {
            install_handler();
        }
    }

void nlohmann::crow::install_handler() {
    if (existing_termination_handler == nullptr) {
        existing_termination_handler = std::get_terminate();
        std::set_terminate(&new_termination_handler);

        // we remember this client, because we will use it to report
        // uncaught exceptions with it
        m_client_that_installed_termination_handler = this;
    }
}

void nlohmann::crow::capture_message(const std::string& message, const json& attributes) {
    if (!m_enabled) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_payload_mutex);
    m_payload["message"] = message;
    m_payload["event_id"] = nlohmann::crow_utilities::generate_uuid();
    m_payload["timestamp"] = nlohmann::crow_utilities::get_iso8601();

    if (attributes.is_object()) {
        // logger
        auto logger = attributes.find("logger");
        if (logger != attributes.end()) {
            m_payload["logger"] = *logger;
        }

        // level
        m_payload["level"] = attributes.value("level", "error");

        // context
        auto context = attributes.find("context");
        if (context != attributes.end()) {
            merge_context(*context);
        }

        // extra
        auto extra = attributes.find("extra");
        if (extra != attributes.end()) {
            m_payload["extra"] = *extra;
        }
    }

    enqueue_post();
}


void nlohmann::crow::capture_exception(const std::exception& exception, const char* file, const unsigned int line, const json& context, const bool handled) {
    if (!m_enabled) {
        return;
    }

    std::stringstream thread_id;
    thread_id << std::this_thread::get_id();

    const std::string& type = crow_utilities::pretty_name(typeid(exception).name());
    const std::string ex_position = std::string(file) + ":" + std::to_string(line);
    m_payload["exception"]["values"].push_back({
        { "type", type },
        { "value", exception.what() },
        { "module", crow_utilities::pretty_name(typeid(exception).name(), true) },
        { "mechanism", {
            { "type", type },
            { "handled", handled },
            { "description", handled ? "handled exception" : "unhandled exception" },
            { "data", ex_position }
        }},
        { "stacktrace", {
            { "frames", crow_utilities::get_backtrace() }
        }},
        { "thread_id", thread_id.str() }
    });
    m_payload["event_id"] = crow_utilities::generate_uuid();
    m_payload["timestamp"] = nlohmann::crow_utilities::get_iso8601();
    m_payload["platform"] = "native";

    // add given context
    merge_context(context);

    enqueue_post();

    // we want to support at most m_maximal_jobs running jobs
    std::lock_guard<std::mutex> lock(m_payload_mutex);
    m_jobs.reserve(m_maximal_jobs);
}

void nlohmann::crow::add_breadcrumb(const std::string& message = "", const json& attributes) {
    if (!shiro::config::bancho::enable_breadcrumb) {
        return;
    }

    if (m_payload["breadcrumb"]["values"].size() > shiro::config::bancho::breadcrumb_limit) {
        enqueue_post(true);
    }

    json breadcrumb = {
        { "event_id", crow_utilities::generate_uuid() },
        { "level", "info" },
        { "type", "default" },
        { "category", "log" },
        { "timestamp", crow_utilities::get_timestamp() }
    };

    if (!message.empty()) {
        breadcrumb["message"] = message;
    }

    if (attributes.is_object()) {
        // type
        auto type = attributes.find("type");
        if (type != attributes.end()) {
            breadcrumb["type"] = *type;
        }

        // level
        auto level = attributes.find("level");
        if (level != attributes.end()) {
            breadcrumb["level"] = *level;
        }

        // category
        auto category = attributes.find("category");
        if (category != attributes.end()) {
            breadcrumb["category"] = *category;
        }

        // data
        auto data = attributes.find("data");
        if (data != attributes.end()) {
            breadcrumb["data"] = *data;
        }
    }

    std::unique_lock<std::mutex> lock(m_payload_mutex);
    m_payload["breadcrumbs"]["values"].push_back(std::move(breadcrumb));
}

void nlohmann::crow::remove_breadcrumbs(size_t amount) noexcept {
    std::unique_lock<std::mutex> lock(m_payload_mutex);

    const size_t current_size = m_payload["breadcrumbs"]["values"].size();
    if (current_size <= amount) {
        m_payload["breadcrumbs"]["values"].clear();
        return;
    }

    // Every new value adds from back, so we remove from begin
    m_payload["breadcrumbs"]["values"].erase(m_payload["breadcrumbs"]["values"].begin(), m_payload["breadcrumbs"]["values"].begin() + amount);
}

std::string nlohmann::crow::get_last_event_id() const {
    if (not m_posts) {
        return "";
    }

    std::unique_lock<std::mutex> lock(m_jobs_mutex);
    if (not m_jobs.empty() and m_jobs.back().valid()) {
        m_last_event_id = m_jobs.back().get();
        m_jobs.clear();
    }

    assert(not m_last_event_id.empty());
    return m_last_event_id;
}

const json& nlohmann::crow::get_context() const {
    return m_payload;
}

void nlohmann::crow::add_user_context(const json& data) {
    std::lock_guard<std::mutex> lock(m_payload_mutex);
    m_payload["user"].update(data);
}

void nlohmann::crow::add_tags_context(const json& data) {
    std::lock_guard<std::mutex> lock(m_payload_mutex);
    m_payload["tags"].update(data);
}

void nlohmann::crow::add_request_context(const json& data) {
    std::lock_guard<std::mutex> lock(m_payload_mutex);
    m_payload["request"].update(data);
}

void nlohmann::crow::add_extra_context(const json& data) {
    std::lock_guard<std::mutex> lock(m_payload_mutex);
    m_payload["extra"].update(data);
}

void nlohmann::crow::merge_context(const json& context) {
    if (context.is_object()) {
        std::lock_guard<std::mutex> lock(m_payload_mutex);
        for (const auto& el : context.items()) {
            const std::string& k = el.key();
            if (k == "user" or k == "request" or k == "extra" or k == "tags") {
                m_payload[k].update(el.value());
            }
        }
    }
}

void nlohmann::crow::clear_context() {
    std::lock_guard<std::mutex> lock(m_payload_mutex);

    m_payload.clear();
    m_payload["platform"] = "c";
    m_payload["sdk"]["name"] = "crow";
    m_payload["sdk"]["version"] = "0.0.6";
    m_payload["sdk"]["integrations"] = { "shiro" }; // Tell sentry we're using Shiro specific changes

    // add context: app
#if defined(_DEBUG) && !defined(NDEBUG)
    m_payload["contexts"]["app"]["build_type"] = "debug";
#else
    m_payload["contexts"]["app"]["build_type"] = "release";
#endif

    // add context: device
    m_payload["contexts"]["device"]["arch"] = shiro::native::system_info::get_architecture();
    m_payload["contexts"]["device"]["name"] = shiro::native::system_info::get_host_name();
    m_payload["contexts"]["device"]["model"] = shiro::native::system_info::get_hw_model();
    m_payload["contexts"]["device"]["memory_size"] = shiro::native::system_stats::get_total_physical_memory();

    // add context: os
#if defined(_WIN32)
    m_payload["contexts"]["os"]["name"] = "windows";
#elif defined(__APPLE__)
    m_payload["contexts"]["os"]["name"] = "macos";
#elif defined(__linux__)
    m_payload["contexts"]["os"]["name"] = "linux";
#else
    m_payload["contexts"]["os"]["name"] = "unknown";
#endif

    // important
    m_payload["contexts"]["os"]["version"] = shiro::native::system_info::get_os_version();
    m_payload["contexts"]["os"]["build"] = shiro::native::system_info::get_os_build();
    m_payload["contexts"]["os"]["kernel_version"] = shiro::native::system_info::get_os_version();

// Utilities for strings in compiler
#define str_helper(x) #x
#define str(x) str_helper(x)

#if defined(_MSC_VER)
    m_payload["contexts"]["runtime"]["name"] = "msvc";
    m_payload["contexts"]["runtime"]["version"] = str(_MSC_FULL_VER) " (" str(_MSC_VER) ")";
#elif defined(__clang__)
    m_payload["contexts"]["runtime"]["name"] = "clang";
    m_payload["contexts"]["runtime"]["version"] = __clang_version__;
#elif defined(__GNUC__)
    m_payload["contexts"]["runtime"]["name"] = "gcc";
    m_payload["contexts"]["runtime"]["version"] = str(__GNUC__) "." str(__GNUC_MINOR__) "." str(__GNUC_PATCHLEVEL__);
#endif

// Undefine utilities for strings defined above
#undef str_helper
#undef str

    // add context: user
    const char* user = getenv("USER");
    if (user == nullptr) {
        user = getenv("USERNAME");
    }

    if (user) {
        m_payload["user"]["id"] = std::string(user) + "@" + shiro::native::system_info::get_host_name();
        m_payload["user"]["username"] = user;
    }
}

curl_wrapper::response nlohmann::crow::post(const json& payload) const {
    curl_wrapper curl;

    // add security header
    std::string security_header = "X-Sentry-Auth: Sentry sentry_version=7,sentry_client=crow-shiro/0.0.6,sentry_timestamp=";
    security_header += std::to_string(crow_utilities::get_timestamp());
    security_header += ",sentry_key=" + m_public_key;

    if (!m_secret_key.empty()) {
        security_header += ",sentry_secret=" + m_secret_key;
    }

    curl.set_header(security_header.c_str());

    return curl.post(m_store_url, payload, true);
}

void nlohmann::crow::enqueue_post(bool send_independently) {
    if (not m_enabled) {
        return;
    }

    // https://develop.sentry.dev/sdk/features/#event-sampling
    const auto rand = send_independently ? -1 : crow_utilities::get_random_number(0, 99);
    if (rand >= m_sample_rate) {
        return;
    }

    // we want to change the job list
    std::unique_lock<std::mutex> lock_jobs(m_jobs_mutex);

    // remember we made a post and now can rely on a last id
    m_posts = true;

    // enforce maximal number of running jobs
    if (m_jobs.size() == m_maximal_jobs) {
        // clearing the vector of futures means waiting for their result; we do
        // not need to save an event id, because we will add another post job
        // below
        m_jobs.clear();
    }

    // add the new job
    m_jobs.push_back(shiro::thread::curl_operations.push([this]() {
        this->wait_rate_limit(m_rate_limit_timer);
        curl_wrapper::response response = post(m_payload);

        {
            std::unique_lock<std::mutex> lock_payload(m_payload_mutex);
            m_payload["breadcrumb"]["values"].clear();
        }

        if (response.status_code == 429) {
            const auto& header = response.headers.find("Retry-After");
            if (header != response.headers.end()) {
                this->handle_rate_limit(*header);
            }
        }

        nlohmann::json result = json::parse(response.data, nullptr, false);
        if (result.is_discarded()) {
            return std::string("-1");
        }

        return result["id"].is_string() ? result["id"].get<std::string>() : "-1";
    }));
}

void nlohmann::crow::wait_rate_limit(const std::chrono::seconds& wait) {
    std::this_thread::sleep_for(wait);

    std::lock_guard<std::mutex> m_lock(m_rate_limit_mutex);
    if (m_rate_limit_timer.count() > 0) {
        m_rate_limit_timer -= wait;
    }
}

void nlohmann::crow::handle_rate_limit(const std::pair<std::string, std::string>& header) {
    const std::string& str_time = header.second;
    int32_t time = std::atoi(str_time.c_str());

    std::lock_guard<std::mutex> m_lock(m_rate_limit_mutex);
    m_rate_limit_timer += std::chrono::seconds(time);
}

void nlohmann::crow::new_termination_handler() {
    if (m_client_that_installed_termination_handler == nullptr) {
        return;
    }

    auto current_ex = std::current_exception();
    if (current_ex) {
        m_client_that_installed_termination_handler->add_breadcrumb("uncaught exception", {
            { "type", "exception" },
            { "level", "critical" }
        });

        try {
            std::rethrow_exception(current_ex);
        }
        catch (const std::exception& e) {
            m_client_that_installed_termination_handler->capture_exception(e, nullptr, false, "stack unwind", 0);
        }
    }

    m_client_that_installed_termination_handler->existing_termination_handler();
}
