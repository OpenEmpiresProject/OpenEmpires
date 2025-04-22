#ifndef LOGGER_H
#define LOGGER_H

#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

#include <string>

namespace utils
{
static void initLogger(const std::string& filename)
{
    std::string pattern = "[%T][%l] %t: %v";
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_st>();
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_st>(filename, true);
    console_sink->set_level(spdlog::level::debug);
    file_sink->set_level(spdlog::level::debug);
    console_sink->set_pattern(std::string("%^") + pattern + "%$");
    file_sink->set_pattern(pattern);
    auto logger = new spdlog::logger("multi_sink", {console_sink, file_sink});
    auto loggerSharePtr = std::shared_ptr<spdlog::logger>(logger);

    spdlog::set_default_logger(loggerSharePtr);
    spdlog::set_level(spdlog::level::debug);
}

} // namespace utils

#endif // LOGGER_H