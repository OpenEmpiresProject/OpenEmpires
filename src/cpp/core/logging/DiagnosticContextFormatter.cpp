#include "DiagnosticContextFormatter.h"

#include "logging/DiagnosticContext.h"

#include <string>

using namespace core;

void DiagnosticContextFormatter::format(const spdlog::details::log_msg& msg,
                                        const std::tm&,
                                        spdlog::memory_buf_t& dest)
{
    std::string prefix = DiagnosticContext::getInstance().format();
    if (!prefix.empty())
    {
        fmt::format_to(std::back_inserter(dest), "[{}] ", prefix);
    }
}

std::unique_ptr<spdlog::custom_flag_formatter> DiagnosticContextFormatter::clone() const
{
    return std::make_unique<DiagnosticContextFormatter>();
}
