#include <source_location>
#include <functional>
#include <iostream>
#include <format>
#include <print>


enum log_level {
    ERROR = 0,
    EXCEPTION = 1,
    WARNING = 2,
    INFO = 3,
};

// any string marked higher than or equal to this level is printed
static constexpr auto LOG_LEVEL = log_level::EXCEPTION;
constexpr std::string_view prompts[4] = {
    "\033[1;41;37m{}> \033[1;49;41m{}> \033[3;49;37min line: \033[0m {}\n"
    "\033[1;35;43m{}> \033[1;49;35m{}> \033[3;49;37min line: \033[0m {}\n"
    "\033[1;43;46m{}> \033[1;49;43m{}> \033[3;49;37min line: \033[0m {}\n"
    "\033[1;42;37m{}> \033[1;49;42m{}> \033[3;49;37min line: \033[0m {}\n"
};


template<typename... Targs>
struct fmt_args_holder {
    std::tuple<std::remove_cvref_t<Targs>...> storage;
    std::format_args args;

    template<typename... T>
    explicit fmt_args_holder(T&& ...a)
        : storage(std::forward<T>(a)...),
          args(std::apply([](auto&... x) {
            return std::make_format_args(x...);
          }, storage)) {}
};

template<typename... Targs>
[[nodiscard]] auto make_format_args_safe(Targs&& ...args) {
    return fmt_args_holder<Targs...>(std::forward<Targs>(args)...).args;
}

auto message(log_level level, std::source_location loc = std::source_location::current()) {
    auto color = std::vformat(
        prompts[static_cast<int>(level)],
        make_format_args_safe(loc.file_name(), loc.function_name(), loc.line(), std::string("{}"))
    );

    return [color](std::string s, auto&& ...rest) -> std::string {
        return std::vformat(color, make_format_args_safe(s, std::forward<decltype(rest)>(rest)...));
    };
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
    explicit NullLogger();
    explicit NullLogger(...);
};

template <log_level Lv>
class BaseLogger {
private:
    static std::string buf;
    static std::source_location loc;
    using Outbuf = std::conditional<(Lv >= EXCEPTION), LogStderr, LogStdout>;
public:
    BaseLogger &operator () (std::string s, auto&& ...rest) {
        buf = message(Lv, loc)(s, std::forward<decltype(rest)>(rest)...);
        std::print(Outbuf::out, buf);
        return *this;
    }
    BaseLogger &operator () (const char *s, auto&& ...rest) {
        buf = message(Lv, loc)(std::string(s), std::forward<decltype(rest)>(rest)...);
        std::print(Outbuf::out, buf);
        return *this;
    }
    const std::string &operator * () const {
        return buf;
    }
    explicit BaseLogger(std::source_location sloc, std::string s, auto&& ...rest) {
        loc = sloc;
        (*this)(s, std::forward<decltype(rest)>(rest)...);
    }
    explicit BaseLogger(std::source_location sloc, const char *s, auto&& ...rest) {
        loc = sloc;
        (*this)(s, std::forward<decltype(rest)>(rest)...);
    }
    explicit BaseLogger() = default;
};

template <enum log_level Lv>
using Logger = std::conditional_t<(Lv >= LOG_LEVEL), BaseLogger<Lv>, NullLogger>;

#define LOG_ERROR(s, ...)     Logger<ERROR>(std::source_location::current(), s, ##__VA_ARGS__)
#define LOG_EXCEPTION(s, ...) Logger<EXCEPTION>(std::source_location::current(), s, ##__VA_ARGS__)
#define LOG_WARNING(s, ...)   Logger<WARNING>(std::source_location::current(), s, ##__VA_ARGS__)
#define LOG_INFO(s, ...)      Logger<INFO>(std::source_location::current(), s, ##__VA_ARGS__)

#define LOG(lv)               LOG_##lv
