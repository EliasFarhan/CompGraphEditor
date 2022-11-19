#pragma once
#include <string>
#include <string_view>
#include <vector>

namespace gpr5300
{
    void Error(const char* file, int line, std::string_view msg);
    #define LogError(msg) Error(__FILE__, __LINE__, msg)

    void Warning(const char* file, int line, std::string_view msg);
    #define LogWarning(msg) Warning(__FILE__, __LINE__, msg)

    void Debug(const char* file, int line, std::string_view msg);
    #define LogDebug(msg) Debug(__FILE__, __LINE__, msg)

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
} // namespace gpr5300
