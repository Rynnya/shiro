/*
    Naga - simple logging system with signal handlers, based on fmt (https://github.com/fmtlib/fmt)

    Created by Rynnya
    Licensed under the MIT License <http://opensource.org/licenses/MIT>.
    Copyright (c) 2021-2022 Rynnya

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

#include "naga.hh"

#include "fmt/chrono.hh"
#include "fmt/ostream.hh"

#include <fstream>
#include <thread>
#include <unordered_map>
#include <shared_mutex>

/* Begin of Variables */

naga::config naga::configuration = naga::config();

/*
    The reason to separate this variables and detail variables is because detail variables
    only used inside printing functions and signal handlers, and i'm don't want to garbage namespace even more
*/

namespace naga::declarations {

    // Declarations
    struct callback_ {
        std::string name = {};
        log_handler_t cb = nullptr;
        log_level level = log_level::info;
        std::any user_info = {};
    };

    // Constants
    const std::chrono::system_clock::time_point startup_time_ = std::chrono::system_clock::now();
    const thread_local std::hash<std::thread::id> thread_hash_ = std::hash<std::thread::id>();

    // File output
    bool file_initialized_ = false;
    std::ofstream log_file_;
    std::thread flush_thread_;
    bool required_flush_ = false;

    // Callbacks
    fatal_handler_t fatal_handler_ = nullptr;
    std::vector<naga::declarations::callback_> callbacks_ = {};
    std::shared_mutex callback_mtx_;
    
}

/* End of Variables */

/* Begin of Internal functions and Variables */

namespace naga::internals {

    bool can_log(log_level level) {
        return (naga::configuration.logging_level & static_cast<uint8_t>(level)) || (naga::configuration.logging_level & static_cast<uint8_t>(log_level::all));
    }

    std::string format(const fmt::text_style& style, const char* type, const log_message& message) {
        return fmt::format(
            style,
            "{} | [{:>8d}s] ({}) | {:>24}:{:<4d} | {}: {}\n",
            message.date,
            message.seconds,
            message.thread_hash,
            message.file, message.line,
            type, message.message
        );
    }
}

/* End of Internal functions and Variables */

/* Begin of Declarations */

void naga::log_message::fill() {
    this->thread_hash = naga::get_thread_name();
    this->date = fmt::format("{:%Y-%m-%d %H:%M:%S}", fmt::localtime(std::chrono::system_clock::now()));
    this->seconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - declarations::startup_time_).count();
}

bool naga::init(naga::config cfg) {
    initialize_file(cfg.filepath, cfg.filepath_with_time);

    if (cfg.own_flush_thread) {
        initialize_flush(cfg.milliseconds_to_flush);
    }

    if (cfg.install_signals) {
        return install_signal_handlers(cfg.signals);
    }

    return true;
}

void naga::initialize_file(const std::string& filename, bool with_time) {
    std::string formatted;

    // User might misunderstand with_time variable and didn't leave argument inside filename
    try {
        formatted = with_time ? fmt::format(filename, declarations::startup_time_) : filename;
    }
    catch (...) {
        LOG_F(WARNING, "Please switch with_time argument in initialize_file() function or add format argument for time!");
        formatted = filename;
    }

    naga::declarations::log_file_ = std::ofstream(formatted, std::ios::out | std::ios::app | std::ios::ate);
    declarations::file_initialized_ = true;
}

void naga::initialize_flush(size_t milliseconds) {
    if (!declarations::file_initialized_ || milliseconds == 0) {
        return;
    }

    std::chrono::milliseconds timeout = std::chrono::milliseconds(milliseconds);
    declarations::flush_thread_ = std::thread([&, timeout]() {
        while (true) {
            std::this_thread::sleep_for(timeout);

            if (declarations::required_flush_) {
                naga::flush();
            }
        }
    });
}

void naga::flush() {
    if (!declarations::file_initialized_) {
        return;
    }

    return static_cast<void>(naga::declarations::log_file_.flush());
}

void naga::adjust_log_level(config& cfg, log_level level) {
    cfg.logging_level |= static_cast<uint8_t>(level);
}

void naga::reset_log_level(config& cfg, log_level level) {
    cfg.logging_level = static_cast<uint8_t>(level);
}

void naga::abort_on_checks(config& cfg, bool state) {
    cfg.abort_on_checks = state;
}

void naga::set_fatal_handler(fatal_handler_t handler) {
    naga::declarations::fatal_handler_ = handler;
}

naga::fatal_handler_t naga::get_fatal_handler() {
    return naga::declarations::fatal_handler_;
}

void naga::add_callback(const std::string& name, log_handler_t cb, std::any user_data, log_level level) {
    if (cb == nullptr) {
        LOG_F(WARNING, "Don't install nullptr's callbacks!");
        return;
    }

    std::unique_lock<std::shared_mutex> lock(naga::declarations::callback_mtx_);
    declarations::callbacks_.push_back(naga::declarations::callback_{ name, cb, level, user_data });
}

void naga::remove_callback(const std::string& name) {
    std::unique_lock<std::shared_mutex> lock(naga::declarations::callback_mtx_);
    for (auto it = declarations::callbacks_.begin(); it != declarations::callbacks_.end(); it++) {
        if (it->name == name) {
            declarations::callbacks_.erase(it);
            return;
        }
    }

    LOG_F(WARNING, "Callback with name '{}' wasn't found.", name);
}

std::string naga::get_thread_name() {
    return fmt::format("{:>#18x}", declarations::thread_hash_(std::this_thread::get_id()));
}

/* Begin of Detail Functions */ 

const char* naga::detail::get_filename(const char* path) {
    for (auto ptr = path; *ptr; ++ptr) {
        if (*ptr == '/' || *ptr == '\\') {
            path = ptr + 1;
        }
    }
    return path;
}


void naga::detail::write(const fmt::text_style& style, const char* type, const naga::log_message& message) {
    if (!internals::can_log(message.level)) {
        return;
    }

    const std::string formatted = naga::internals::format(style, type, message);

    if (naga::declarations::log_file_) {
        fmt::print(naga::declarations::log_file_, formatted);
        declarations::required_flush_ = true;
    }

    fmt::print(formatted);

    std::shared_lock<std::shared_mutex> lock(naga::declarations::callback_mtx_);
    for (naga::declarations::callback_& record : declarations::callbacks_) {
        if (static_cast<uint8_t>(record.level) <= static_cast<uint8_t>(message.level)) {
            record.cb(record.user_info, message);
        }
    }
}

void naga::detail::output_to_file(const naga::log_message& message) {
    if (naga::declarations::log_file_) {
        fmt::print(naga::declarations::log_file_, naga::internals::format(naga::detail::info_color, "FILE_ONLY", message));
        declarations::required_flush_ = true;
    }
}

void naga::detail::call_fatal_handler(const log_message& message) {
    if (naga::declarations::fatal_handler_ != nullptr) {
        naga::declarations::fatal_handler_(message);
    }
}

/* End of Detail Functions */

/* End of Declarations */

/* Begin of System-Based functions */

#if defined(_WIN32)

#include <csignal>
#include <cstdlib>

namespace naga::system {

    void write_to(const char* data) {
        return static_cast<void>(fputs(data, stderr));
    }

    const char* get_signal_name(int signal_number) {
        switch (signal_number) {
            case SIGABRT:
                return "SIGABRT";
            case SIGFPE:
                return "SIGFPE";
            case SIGILL:
                return "SIGILL";
            case SIGINT:
                return "SIGINT";
            case SIGSEGV:
                return "SIGSEGV";
            case SIGTERM:
                return "SIGTERM";
            default:
                return "UNKNOWN";
        }
    }

    void signal_handler(int signal_number) {
        const char* signal_name = get_signal_name(signal_number);

        write_to(static_cast<const char*>(naga::detail::bold));
        write_to(static_cast<const char*>(naga::detail::fatal_color));

        write_to("\n");
        write_to("Naga caught a signal: ");
        write_to(signal_name);
        write_to("\n\x1b[0m"); // Reset
    }

}

bool naga::install_signal_handlers(naga::signal_config signals) {
    naga::configuration.signals = signals;

    if (signals.sigabrt) {
        CHECK_NO_ASSERT_F(signal(SIGABRT, naga::system::signal_handler) != SIG_ERR, "Failed to install handler for SIGABRT");
    }

    // SIGBUS don't exist on Windows

    if (signals.sigfpe) {
        CHECK_NO_ASSERT_F(signal(SIGFPE, naga::system::signal_handler) != SIG_ERR, "Failed to install handler for SIGFPE");
    }
    if (signals.sigill) {
        CHECK_NO_ASSERT_F(signal(SIGILL, naga::system::signal_handler) != SIG_ERR, "Failed to install handler for SIGILL");
    }
    if (signals.sigint) {
        CHECK_NO_ASSERT_F(signal(SIGINT, naga::system::signal_handler) != SIG_ERR, "Failed to install handler for SIGINT");
    }
    if (signals.sigsegv) {
        CHECK_NO_ASSERT_F(signal(SIGSEGV, naga::system::signal_handler) != SIG_ERR, "Failed to install handler for SIGSEGV");
    }
    if (signals.sigterm) {
        CHECK_NO_ASSERT_F(signal(SIGTERM, naga::system::signal_handler) != SIG_ERR, "Failed to install handler for SIGTERM");
    }

    return true;
}

void naga::detail::disable_abort_handler() {
    CHECK_NO_ASSERT_F(signal(SIGABRT, SIG_DFL) != SIG_ERR, "Failed to disable handler for SIGABRT, this will cause a deadlock!");
}

#endif

#if defined(__linux__) || defined(__APPLE__)

#include <csignal>
#include <cstdlib>

#include <unistd.h>

namespace naga::system {

    void write_to(const char* data) {
        return static_cast<void>(::write(STDERR_FILENO, data, strlen(data)));
    }

    const char* get_signal_name(int signal_number) {
        switch (signal_number) {
            case SIGABRT:
                return "SIGABRT";
            case SIGBUS:
                return "SIGBUS";
            case SIGFPE:
                return "SIGFPE";
            case SIGILL:
                return "SIGILL";
            case SIGINT:
                return "SIGINT";
            case SIGSEGV:
                return "SIGSEGV";
            case SIGTERM:
                return "SIGTERM";
            default:
                return "UNKNOWN";
        }
    }

    void call_default_signal_handler(int signal_number) {
        struct sigaction sig_action;
        memset(&sig_action, 0, sizeof(sig_action));
        sigemptyset(&sig_action.sa_mask);
        sig_action.sa_handler = SIG_DFL;
        sigaction(signal_number, &sig_action, nullptr);
        kill(getpid(), signal_number);
    }

    void signal_handler(int signal_number, siginfo_t*, void*) {
        const char* signal_name = get_signal_name(signal_number);

        write_to(static_cast<const char*>(naga::detail::bold));
        write_to(static_cast<const char*>(naga::detail::fatal_color));

        write_to("\n");
        write_to("Naga caught a signal: ");
        write_to(signal_name);
        write_to("\n\x1b[0m"); // Reset

        call_default_signal_handler(signal_number);
    }

}

bool naga::install_signal_handlers(naga::signal_config signals) {
    naga::configuration.signals = signals;

    struct sigaction sig_action;
    memset(&sig_action, 0, sizeof(sig_action));
    sigemptyset(&sig_action.sa_mask);
    sig_action.sa_flags |= SA_SIGINFO;
    sig_action.sa_sigaction = &naga::system::signal_handler;

    if (signals.sigabrt) {
        CHECK_NO_ASSERT_F(sigaction(SIGABRT, &sig_action, nullptr) != -1, "Failed to install handler for SIGABRT");
    }
    if (signals.sigbus) {
        CHECK_NO_ASSERT_F(sigaction(SIGBUS, &sig_action, nullptr) != -1, "Failed to install handler for SIGBUS");
    }
    if (signals.sigfpe) {
        CHECK_NO_ASSERT_F(sigaction(SIGFPE, &sig_action, nullptr) != -1, "Failed to install handler for SIGFPE");
    }
    if (signals.sigill) {
        CHECK_NO_ASSERT_F(sigaction(SIGILL, &sig_action, nullptr) != -1, "Failed to install handler for SIGILL");
    }
    if (signals.sigint) {
        CHECK_NO_ASSERT_F(sigaction(SIGINT, &sig_action, nullptr) != -1, "Failed to install handler for SIGINT");
    }
    if (signals.sigsegv) {
        CHECK_NO_ASSERT_F(sigaction(SIGSEGV, &sig_action, nullptr) != -1, "Failed to install handler for SIGSEGV");
    }
    if (signals.sigterm) {
        CHECK_NO_ASSERT_F(sigaction(SIGTERM, &sig_action, nullptr) != -1, "Failed to install handler for SIGTERM");
    }

    return true;
}

void naga::detail::disable_abort_handler() {
    CHECK_NO_ASSERT_F(signal(SIGABRT, SIG_DFL) != SIG_ERR, "Failed to disable handler for SIGABRT, this will cause a deadlock!");
}

#endif

/* End of System-Based functions */