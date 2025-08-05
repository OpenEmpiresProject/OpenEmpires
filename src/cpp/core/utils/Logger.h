#ifndef LOGGER_H
#define LOGGER_H

#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

#include <string>

#ifndef NDEBUG

// In a debug build, 'spam' will expand to a spdlog::trace call.
// This uses the SPDLOG_LOGGER_TRACE macro from spdlog, which is a bit safer
// and more explicit than a direct call to spdlog::trace.
#define spam(...) spdlog::trace(__VA_ARGS__)

#else // If not in a debug build (e.g., NDEBUG is defined)

// In a release build, 'spam' will be a no-op. This results in zero runtime
// overhead. Not even argument evaluation.
#define spam(...) (void) 0
#endif

namespace core
{
static void initLogger(const std::string& filename)
{
    std::string pattern = "[%T][%l] %t: %v";
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_st>();
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_st>(filename, true);
    console_sink->set_level(spdlog::level::trace);
    file_sink->set_level(spdlog::level::trace);
    console_sink->set_pattern(std::string("%^") + pattern + "%$");
    file_sink->set_pattern(pattern);
    auto logger = new spdlog::logger("multi_sink", {console_sink, file_sink});
    auto loggerSharePtr = std::shared_ptr<spdlog::logger>(logger);

    spdlog::set_default_logger(loggerSharePtr);
    spdlog::set_level(spdlog::level::debug);
}

} // namespace core

#endif // LOGGER_H