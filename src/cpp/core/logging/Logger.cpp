#include "logging/Logger.h"

#include "DiagnosticContextFormatter.h"
#include "ThreadAliasFormatter.h"
#include "logging/tcp_sink.h"
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

void core::initLogger(const std::string& filename)
{
    std::string pattern = "[%H:%M:%S.%e][%l][%y]:%x %v";
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

    spdlog::sinks::tcp_sink_config tcpConfig("localhost", 5000);
    tcpConfig.lazy_connect = true;
    tcpConfig.timeout_ms = 10;
    auto tcpSink = std::make_shared<spdlog::sinks::tcp_sink_mt>(tcpConfig);
    tcpSink->set_level(spdlog::level::trace);
    auto tcpCustomFormatter = spdlog::details::make_unique<spdlog::pattern_formatter>();
    tcpCustomFormatter->add_flag<DiagnosticContextFormatter>('x')
        .add_flag<ThreadAliasFormatter>('y')
        .set_pattern(pattern);
    tcpSink->set_formatter(std::move(tcpCustomFormatter));

    spdlog::sinks_init_list sinks{consoleSink, fileSink, tcpSink};
    // Nothing special about 8192 for us, but it is the default value of spdlog. Hence using same.
    // It is important to have thread count as 1 to maintain the order of logs.
    spdlog::init_thread_pool(8192, 1);
    auto logger = std::make_shared<spdlog::async_logger>("multi_sink", sinks, spdlog::thread_pool(),
                                                         spdlog::async_overflow_policy::block);

    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::debug);
}
