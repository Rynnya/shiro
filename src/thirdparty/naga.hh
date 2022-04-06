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

#ifndef _NAGA_HEADER
#define _NAGA_HEADER

#include <any>
#include <cstdint>
#include <string>

#include "fmt/format.hh"
#include "fmt/color.hh"

namespace naga {

    struct signal_config {
        // Will Naga handle SIGABRT?
        bool sigabrt = true;

        // Will Naga handle SIGBUS? (Unix and Linux systems only, do nothing on Windows)
        bool sigbus = true;

        // Will Naga handle SIGFPE?
        bool sigfpe = true;

        // Will Naga handle SIGILL?
        bool sigill = true;

        // Will Naga handle SIGINT?
        bool sigint = true;

        // Will Naga handle SIGSEGV?
        bool sigsegv = true;

        // Will Naga handle SIGTERM?
        bool sigterm = true;
    };

    enum class log_level : uint8_t {
        all = 1,
        info = 2,
        warning = 4,
        error = 8,
        assertions = 16,
        fatal = 32
    };

    struct config {
        std::string filepath = "{:%Y-%m-%d_%H-%M-%S}.log";
        bool filepath_with_time = true;

        bool own_flush_thread = true;
        size_t milliseconds_to_flush = 300;

        bool install_signals = true;
        signal_config signals = signal_config();

        uint8_t logging_level = static_cast<uint8_t>(naga::log_level::all);

        bool abort_on_checks = false;
        bool abort_on_fatal = true;
    };

    // Forward declaration
    class log_message;

    // Including <functional> will slow down compile time drastically
    typedef void (*fatal_handler_t)(const log_message&);
    typedef void (*log_handler_t)(std::any&, const log_message&);

    extern config configuration;

    // Initializes output file and signal handlers
    bool init(config cfg = naga::configuration);

    // Initializes output file
    void initialize_file(const std::string& filepath, bool with_time = false);
    // Initializes flush thread
    void initialize_flush(size_t milliseconds);

    // Flushes internal log file, can be used to create own flush thread
    void flush();

    // Cross-platform signal handlers
    bool install_signal_handlers(signal_config signals = naga::configuration.signals);

    // Adds log level to current config
    void adjust_log_level(config& cfg, log_level level);
    // Resets log level in current config
    void reset_log_level(config& cfg, log_level level = log_level::all);

    // Will CHECK_F aborts a program?
    void abort_on_checks(config& cfg, bool state = false);

    // Setter for fatal handler
    void set_fatal_handler(fatal_handler_t handler);
    // Getter for fatal handler
    fatal_handler_t get_fatal_handler();

    // Adds your own callback when LOG_F with current or higher log_level called
    void add_callback(const std::string& name, log_handler_t cb, std::any user_data = {}, log_level level = log_level::info);
    // Removes callback with corresponded name, if callback wasn't find - warns and returns
    void remove_callback(const std::string& name);

    // Returns current thread as hashed string
    std::string get_thread_name();

    // Don't use this functions directly, instead use macros
    namespace detail {

        const fmt::text_style info_color = fmt::text_style();
        const fmt::text_style warning_color = fg(fmt::color::orange);
        const fmt::text_style error_color = fg(fmt::color::crimson);

        const fmt::detail::ansi_color_escape<char> bold = fmt::detail::ansi_color_escape<char>(fmt::emphasis::bold);
        const fmt::detail::ansi_color_escape<char> fatal_color = fmt::detail::ansi_color_escape<char>(fmt::color::crimson, "\x1b[38;2;");

        const char* get_filename(const char* path);
        void write(const fmt::text_style& style, const char* type, const naga::log_message& message);
        void output_to_file(const naga::log_message& message);
        void call_fatal_handler(const naga::log_message& message);

        #undef ERROR // Thank you Windows, very cool
        enum class print_types : uint8_t {
            INFO = 0,
            WARNING = 1,
            ERROR = 2,
            FATAL = 3,
            FILE_ONLY = 255 // Used to output ONLY in file, will skip all callbacks, useful when needed to output a lot of one-type information
        };

        // Used to escape deadlock on fatal abort
        void disable_abort_handler();

        template <typename... T>
        void check(bool result, const char* exp, bool terminate, const std::string& format, const char* file, unsigned int line, T&&... args) {
            if (result) { return; }

            const std::string std_exp = fmt::format("EXP {} FAILED", exp);
            naga::detail::write(naga::detail::error_color, std_exp.c_str(), naga::log_message{ format, naga::log_level::info, file, line, std::forward<T>(args)... });

            if (terminate && naga::configuration.abort_on_checks) {
                std::terminate();
            }
        }

        template <typename... T>
        FMT_NORETURN void abort(const std::string& format, const char* file, unsigned int line, T&&... args) {
            naga::detail::write(naga::detail::error_color, "ABORT", naga::log_message{ format, naga::log_level::info, file, line, std::forward<T>(args)... });
            disable_abort_handler();
            std::abort();
        }

        template <print_types P> class printer {
        public:
            template <typename... T>
            static void call(const std::string& format, const char* file, unsigned int line, T&&... args) {};
        };

        template <> template <typename... T>
        void detail::printer<print_types::INFO>::call(const std::string& format, const char* file, unsigned int line, T&&... args) {
            naga::detail::write(naga::detail::info_color, "INFO", naga::log_message{ format, naga::log_level::info, file, line, std::forward<T>(args)... });
        }

        template <> template <typename... T>
        void detail::printer<print_types::WARNING>::call(const std::string& format, const char* file, unsigned int line, T&&... args) {
            naga::detail::write(naga::detail::warning_color, "WARNING", naga::log_message{ format, naga::log_level::info, file, line, std::forward<T>(args)... });
        }

        template <> template <typename... T>
        void detail::printer<print_types::ERROR>::call(const std::string& format, const char* file, unsigned int line, T&&... args) {
            naga::detail::write(naga::detail::error_color, "ERROR", naga::log_message{ format, naga::log_level::info, file, line, std::forward<T>(args)... });
        }

        template <> template <typename... T>
        void detail::printer<print_types::FATAL>::call(const std::string& format, const char* file, unsigned int line, T&&... args) {
            auto message = naga::log_message{ format, naga::log_level::info, file, line, std::forward<T>(args)... };

            naga::detail::call_fatal_handler(message);
            naga::detail::write(naga::detail::error_color, "FATAL", message);

            if (naga::configuration.abort_on_fatal) {
                disable_abort_handler();
                std::abort();
            }
        }

        template <> template <typename... T>
        void detail::printer<print_types::FILE_ONLY>::call(const std::string& format, const char* file, unsigned int line, T&&... args) {
            naga::detail::output_to_file(naga::log_message{ format, naga::log_level::all, file, line, std::forward<T>(args)... });
        }
    }

    class log_message {
    public:
        template <typename... T>
        log_message(const std::string& format_, log_level level_, const char* file_, unsigned int line_, T&&... args_)
            : level(level_)
            , file(naga::detail::get_filename(file_))
            , line(line_) {
            this->message = fmt::format(format_, std::forward<T>(args_)...);
            fill();
        }

    public:
        naga::log_level level;
        const char* file;
        unsigned int line;

        std::string thread_hash = "0x0000000000000000";
        std::string date = "01.01.1970 00:00:00";
        int64_t seconds = 0;

        // User defined message
        std::string message = "";

    private:
        void fill();
    };

    /* 
        Simply output formatted string into consoleand file if opened.
        Example: LOG_F(INFO, "Hello {}!", "World");
    */
    #define LOG_F(TYPE, FORMAT, ...) naga::detail::printer<naga::detail::print_types:: TYPE>::call(FORMAT, __FILE__, __LINE__, ##__VA_ARGS__)

    /*
        Checks result of expression, if fails - terminate a program if config.abort_on_checks is true.
        Example: CHECK_F(some_required_variable == nullptr, "Variable wasn't nullptr: {}", fmt::ptr(some_required_variable));
    */
    #define CHECK_F(EXP, FORMAT, ...) naga::detail::check((EXP), #EXP, true, FORMAT, __FILE__, __LINE__, ##__VA_ARGS__)

    /*
        Checks result of expression, never terminate a program.
        Example: CHECK_NO_ASSERT_F(some_required_variable == nullptr, "Variable wasn't nullptr: {}", fmt::ptr(some_required_variable));
    */
    #define CHECK_NO_ASSERT_F(EXP, FORMAT, ...) naga::detail::check((EXP), #EXP, false, FORMAT, __FILE__, __LINE__, ##__VA_ARGS__)

    /*
        Outputs formatted string and aborts a program.
        Example: ABORT_F("Assertion failed: {} != {}", variable_one, variable_two);
    */
    #define ABORT_F(FORMAT, ...) naga::detail::abort(FORMAT, __FILE__, __LINE__, ##__VA_ARGS__)
}

#endif