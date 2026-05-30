#ifndef LOGGER_H
#define LOGGER_H

#include "logging/DiagnosticContext.h"
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
void initLogger(const std::string& filename);

} // namespace core

#endif // LOGGER_H