#ifndef IONDEBUG_H
#define IONDEBUG_H

#include <cstdlib> // std::abort
#include <fmt/core.h>
#include <string_view>

namespace debug
{
// Base debug_assert function, works only in debug mode. Indicates function never returns.
#ifndef NDEBUG
template <typename... Args>
[[noreturn]] inline void assert_fail(const std::string_view condition,
                                     fmt::format_string<Args...> format_str,
                                     Args&&... args)
{
    fmt::print(stderr, "Assertion failed: {}\nError: {}\n", condition,
               fmt::format(format_str, std::forward<Args>(args)...));
    std::abort();
}

// Debug asserting with string formatted error messages. do-while is to force it to expand properly
// without any issues such as dangling else.
#define debug_assert(cond, fmt_str, ...)                                                           \
    do                                                                                             \
    {                                                                                              \
        if (!(cond))                                                                               \
            ::debug::assert_fail(#cond, fmt_str, ##__VA_ARGS__);                                   \
    } while (false)
#else
// In release builds, the macro does nothing
#define debug_assert(cond, fmt_str, ...) ((void) 0)
#endif

} // namespace debug

#endif