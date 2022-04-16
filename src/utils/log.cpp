#include "utils/log.h"
#include <fmt/format.h>
#include <spdlog/spdlog.h>

namespace gpr5300
{
void Error(const char* file, int line, std::string_view msg)
{
    spdlog::error(fmt::format("{}. File: {}, Line: {}", msg, file, line));
}

void Warning(const char* file, int line, std::string_view msg)
{
    spdlog::warn(fmt::format("{}. File: {}, Line: {}", msg, file, line));
}

void Debug(const char* file, int line, std::string_view msg)
{
    spdlog::info(fmt::format("{}. File: {}, Line: {}", msg, file, line));
}
}
