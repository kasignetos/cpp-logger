#include <source_location>
#include <type_traits>
#include <functional>
#include <iostream>
#include <format>
#include <print>
#include <utility>


enum log_level {
    ERROR = 0,
    EXCEPTION = 1,
    WARNING = 2,
    INFO = 3,
};

// any string marked higher than or equal to this level is printed
static constexpr auto LOG_LEVEL = log_level::EXCEPTION;
constexpr std::string_view prompts[4] = {
    "\033[1;42;37m{}> \033[1;49;42m{}> \033[3;49;37min line: {}\033[0m {}\n\0"
    "\033[1;43;46m{}> \033[1;49;43m{}> \033[3;49;37min line: {}\033[0m {}\n\0"
    "\033[1;35;43m{}> \033[1;49;35m{}> \033[3;49;37min line: {}\033[0m {}\n\0"
    "\033[1;41;37m{}> \033[1;49;41m{}> \033[3;49;37min line: {}\033[0m {}\n\0"
};

template<typename... Targs>
// it's NOT `rustly` safe tho ... I promise it's safe anyway
[[nodiscard]] auto make_format_args_safe(Targs&& ...args) {
    return std::make_format_args(std::forward<Targs&>(args)...);
}

struct LogStdout {
    inline static auto &out = std::cout;
};

struct LogStderr {
    inline static auto &out = std::cerr;
};

class NullLogger {
private:
    inline static std::string buf = "";
public:
    NullLogger &operator () (...) noexcept {
        return *this;
    }
    const std::string &operator * () const noexcept {
        return buf;
    }
    explicit NullLogger() = default;
    explicit NullLogger(...) {}
};

template <log_level Lv>
class BaseLogger {
private:
    inline static std::string buf;
    inline static std::source_location loc;
    using Outbuf = std::conditional_t<(Lv <= EXCEPTION), LogStderr, LogStdout>;
    std::string color;
public:
    BaseLogger &operator () (std::string s, auto&& ...rest) {
        buf = std::vformat(s, make_format_args_safe(std::forward<decltype(rest)>(rest)...));
        std::print(Outbuf::out, "{}", std::vformat(color, std::make_format_args(buf)));
        return *this;
    }
    const std::string &operator * () const {
        return buf;
    }
    explicit BaseLogger(std::source_location sloc = std::source_location::current()) {
        loc = sloc;
        color = std::vformat(
            prompts[static_cast<int>(Lv)],
            make_format_args_safe(loc.file_name(), loc.function_name(), loc.line(), "{}")
        );
    }
    explicit BaseLogger() = default;
};

template <enum log_level Lv>
using Logger = std::conditional_t<(Lv <= LOG_LEVEL), BaseLogger<Lv>, NullLogger>;

#define LOG_ERROR     (Logger<ERROR>(std::source_location::current()))
#define LOG_EXCEPTION (Logger<EXCEPTION>(std::source_location::current()))
#define LOG_WARNING   (Logger<WARNING>(std::source_location::current()))
#define LOG_INFO      (Logger<INFO>(std::source_location::current()))

#define LOG(lv)       LOG_##lv
