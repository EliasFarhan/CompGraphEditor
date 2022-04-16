#pragma once
#include <string_view>

namespace gpr5300
{
    void Error(const char* file, int line, std::string_view msg);
    #define LogError(msg) Error(__FILE__, __LINE__, msg)

    void Warning(const char* file, int line, std::string_view msg);
    #define LogWarning(msg) Warning(__FILE__, __LINE__, msg)

    void Debug(const char* file, int line, std::string_view msg);
    #define LogDebug(msg) Debug(__FILE__, __LINE__, msg)
} // namespace gpr5300
