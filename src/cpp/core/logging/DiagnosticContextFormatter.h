#ifndef CORE_DIAGNOSTICCONTEXTFORMATTER_H
#define CORE_DIAGNOSTICCONTEXTFORMATTER_H

#include "spdlog/spdlog.h"

namespace core
{
class DiagnosticContextFormatter : public spdlog::custom_flag_formatter
{
  public:
    void format(const spdlog::details::log_msg& msg,
                const std::tm&,
                spdlog::memory_buf_t& dest) override;

    std::unique_ptr<spdlog::custom_flag_formatter> clone() const override;
};
} // namespace core

#endif // CORE_DIAGNOSTICCONTEXTFORMATTER_H
