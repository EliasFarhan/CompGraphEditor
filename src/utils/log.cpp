#include "utils/log.h"
#include <fmt/format.h>
#include <spdlog/spdlog.h>

namespace gpr5300
{
void LogError(const char* file, int line, std::string_view msg)
{
    spdlog::error(fmt::format("{}. File: {}, Line: {}", msg, file, line));
}

void LogWarning(const char* file, int line, std::string_view msg)
{
    spdlog::warn(fmt::format("{}. File: {}, Line: {}", msg, file, line));
}

void LogDebug(const char* file, int line, std::string_view msg)
{
    spdlog::info(fmt::format("{}. File: {}, Line: {}", msg, file, line));
}
}
