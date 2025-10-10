#include "utils/log.h"
#include <SDL3/SDL_log.h>
#include <format>


namespace core
{

static bool isRecordingLog = false;
static std::vector<Log> logs;

void Error(const char* file, int line, std::string_view msg)
{
    SDL_LogError(SDL_LOG_CATEGORY_ERROR ,"%s. File: %s, Line: %d", msg.data(), file, line);
    if(isRecordingLog)
    {
        const auto fullMsg = std::format("{}. File: {}, Line: {}", msg, file, line);
        logs.push_back({fullMsg,Log::Type::Error});
    }
}

void Warning(const char* file, int line, std::string_view msg)
{
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION ,"%s. File: %s, Line: %d", msg.data(), file, line);
    if(isRecordingLog)
    {
        const auto fullMsg = std::format("{}. File: {}, Line: {}", msg, file, line);
        logs.push_back({fullMsg,Log::Type::Warning});
    }
}

void Debug(const char* file, int line, std::string_view msg)
{
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION ,"%s. File: %s, Line: %d", msg.data(), file, line);
    if(isRecordingLog)
    {
        const auto fullMsg = std::format("{}. File: {}, Line: {}", msg, file, line);
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
