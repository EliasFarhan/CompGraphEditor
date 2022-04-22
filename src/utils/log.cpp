#include "utils/log.h"
#include <fmt/format.h>
#include <spdlog/spdlog.h>

namespace gpr5300
{

static bool isRecordingLog = false;
static std::vector<Log> logs;

void Error(const char* file, int line, std::string_view msg)
{
    const auto fullMsg = fmt::format("{}. File: {}, Line: {}", msg, file, line);
    spdlog::error(fullMsg);
    if(isRecordingLog)
    {
        logs.push_back({fullMsg,Log::Type::Error});
    }
}

void Warning(const char* file, int line, std::string_view msg)
{
    const auto fullMsg = fmt::format("{}. File: {}, Line: {}", msg, file, line);
    spdlog::warn(fullMsg);
    if(isRecordingLog)
    {
        logs.push_back({fullMsg,Log::Type::Warning});
    }
}

void Debug(const char* file, int line, std::string_view msg)
{
    const auto fullMsg = fmt::format("{}. File: {}, Line: {}", msg, file, line);
    spdlog::info(fullMsg);
    if(isRecordingLog)
    {
        logs.push_back({fullMsg,Log::Type::Debug});
    }
}


void EnableLogRecording()
{
    isRecordingLog = true;
}

const std::vector<Log>& GetLogs()
{
    return logs;
}
}
