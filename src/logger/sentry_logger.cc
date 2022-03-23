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

#include "../config/bancho_file.hh"
#include "sentry_logger.hh"

std::shared_ptr<nlohmann::crow> shiro::logging::sentry::client = nullptr;
tsc::TaskScheduler shiro::logging::sentry::scheduler;

void shiro::logging::sentry::init() {
    if (!config::bancho::sentry_integration) {
        LOG_F(WARNING, "Sentry.io error reporting is currently disabled.");
        return;
    }

    client = std::make_shared<nlohmann::crow>(config::bancho::sentry_dsn);
    naga::add_callback("sentry.io", logging::sentry::callback, naga::log_level::info);
    naga::set_fatal_handler(logging::sentry::fatal_callback);

    LOG_F(INFO, "Sentry successfully started.");
    if (config::bancho::enable_breadcrumb) {
        LOG_F(WARNING, "Please note that breadcrumbs might take a lot of RAM!");

        scheduler.Schedule(std::chrono::hours(48), [](tsc::TaskContext ctx) {
            client->remove_breadcrumbs(25);

            ctx.Repeat(std::chrono::hours(48));
        });
        client->add_breadcrumb("Successfully started communication with sentry.io.");
    }
}

void shiro::logging::sentry::callback(std::any& user_data, const naga::log_message& message) {
    switch (message.level) {
        case naga::log_level::error:
        case naga::log_level::assertions:
        case naga::log_level::fatal: {
            client->capture_message(message.message, {
                { "level", verbosity_to_sentry_level(message.level) },
                { "extra",
                    {
                        { "verbosity", static_cast<uint8_t>(message.level) },
                        { "filename", message.file },
                        { "line", message.line },
                        { "date", message.date },
                        { "seconds", message.seconds },
                        { "thread", message.thread_hash }
                    }
                }
            });
            break;
        }
        default: {
            client->add_breadcrumb(message.message, {
                { "level", verbosity_to_sentry_level(message.level) },
                { "data",
                    {
                        { "verbosity", static_cast<uint8_t>(message.level) },
                        { "filename", message.file },
                        { "line", message.line },
                        { "date", message.date },
                        { "seconds", message.seconds },
                        { "thread", message.thread_hash }
                    }
                }
            });
            break;
        }
    }
}

void shiro::logging::sentry::fatal_callback(const naga::log_message& message) {
    client->add_breadcrumb("!! FATAL ERROR OCCURRED, EXITING NOW !!");
    client->capture_message(message.message, {
        { "level", "fatal" },
        { "extra",
            {
                { "verbosity", static_cast<uint8_t>(message.level) },
                { "filename", message.file },
                { "line", message.line },
                { "date", message.date },
                { "seconds", message.seconds },
                { "thread", message.thread_hash }
            }
        }
    });
}

void shiro::logging::sentry::exception(const std::exception &ex, const char* file, const unsigned int line) {
    if (!config::bancho::sentry_integration) {
        return;
    }

    client->capture_exception(ex, file, line);
}

void shiro::logging::sentry::exception(const std::exception_ptr &ptr, const char* file, const unsigned int line) {
    if (!config::bancho::sentry_integration) {
        return;
    }

    try {
        std::rethrow_exception(ptr);
    } catch (const std::exception &ex) {
        client->capture_exception(ex, file, line, nullptr, false);
    }
}

void shiro::logging::sentry::http_request_out(const std::string &url, const std::string &method, int32_t status_code, const std::string &reason) {
    if (!config::bancho::sentry_integration) {
        return;
    }

    json req = {
        { "url", url },
        { "method", method },
        { "status_code", status_code }
    };

    if (!reason.empty()) {
        req["reason"] = reason;
    }

    client->add_breadcrumb("", {
        { "level", "info" },
        { "type", "http" },
        { "data", req }
    });
}

void shiro::logging::sentry::http_request_in(const ::crow::request &request) {
    if (!config::bancho::sentry_integration) {
        return;
    }

    json req = {
        { "url", request.url },
        { "method", ::crow::method_name(request.method) },
        { "data", request.body },
        { "query_string", request.raw_url },
        { "env", {
                "REMOTE_ADDR", request.get_ip_address()
            }
        }
    };

    json headers;

    for (const auto &[key, value] : request.headers) {
        headers[key] = value;
    }

    req["headers"] = headers;

    std::string cookie = request.get_header_value("cookie");

    if (!cookie.empty()) {
        req["cookies"] = cookie;
    }

    client->add_request_context(req);
}

std::string shiro::logging::sentry::verbosity_to_sentry_level(const naga::log_level& verbosity) {
    switch (verbosity) {
        case naga::log_level::all:
        default: {
            return "debug";
        }
        case naga::log_level::info: {
            return "info";
        }
        case naga::log_level::warning: {
            return "warning";
        }
        case naga::log_level::error: {
            return "error";
        }
        case naga::log_level::assertions: {
            return "assertion";
        }
        case naga::log_level::fatal: {
            return "fatal";
        }
    }
}
