#ifndef LOGGER_H
#define LOGGER_H

#include "DiagnosticContextFormatter.h"
#include "ThreadAliasFormatter.h"
#include "logging/DiagnosticContext.h"
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
    std::string pattern = "[%T][%l][%y]:%x %v";
    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_st>();
    consoleSink->set_level(spdlog::level::trace);
    auto consoleCustomFormatter = spdlog::details::make_unique<spdlog::pattern_formatter>();
    consoleCustomFormatter->add_flag<DiagnosticContextFormatter>('x')
        .add_flag<ThreadAliasFormatter>('y')
        .set_pattern(std::string("%^") + pattern + "%$");
    consoleSink->set_formatter(std::move(consoleCustomFormatter));

    auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_st>(filename, true);
    fileSink->set_level(spdlog::level::trace);
    auto fileCustomFormatter = spdlog::details::make_unique<spdlog::pattern_formatter>();
    fileCustomFormatter->add_flag<DiagnosticContextFormatter>('x')
        .add_flag<ThreadAliasFormatter>('y')
        .set_pattern(pattern);
    fileSink->set_formatter(std::move(fileCustomFormatter));

    auto logger = new spdlog::logger("multi_sink", {consoleSink, fileSink});
    auto loggerSharePtr = std::shared_ptr<spdlog::logger>(logger);

    spdlog::set_default_logger(loggerSharePtr);
    spdlog::set_level(spdlog::level::debug);
}

} // namespace core

#endif // LOGGER_H