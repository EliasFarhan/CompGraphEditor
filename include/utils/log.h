#pragma once
#include <string_view>

namespace gpr5300
{
    void LogError(const char* file, int line, std::string_view msg);
    #define Error(msg) LogError(__FILE__, __LINE__, msg)

    void LogWarning(const char* file, int line, std::string_view msg);
    #define Warning(msg) LogWarning(__FILE__, __LINE__, msg)

    void LogDebug(const char* file, int line, std::string_view msg);
    #define Debug(msg) LogDebug(__FILE__, __LINE__, msg) 
} // namespace gpr5300
