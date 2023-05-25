#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace core
{
    void Error(const char* file, int line, std::string_view msg);

    void Warning(const char* file, int line, std::string_view msg);

    void Debug(const char* file, int line, std::string_view msg);

    // Used by the editor to show the logs
    void EnableLogRecording();

    struct Log
    {
        enum class Type
        {
            Error,
            Warning,
            Debug,
            None
        };
        std::string msg;
        Type type = Type::None;
    };
    const std::vector<Log>& GetLogs();
} // namespace core


#define LogError(msg) core::Error(__FILE__, __LINE__, msg)
#define LogWarning(msg) core::Warning(__FILE__, __LINE__, msg)
#define LogDebug(msg) core::Debug(__FILE__, __LINE__, msg)
