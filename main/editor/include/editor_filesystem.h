#pragma once

#include <filesystem>

namespace fs = std::filesystem;

namespace gpr5300
{
std::string GetFileExtension(std::string_view path);
std::string GetFilename(std::string_view path, bool withExtension=true);
fs::file_time_type GetLastTimeWrite(std::string_view path);
bool CopyFile(std::string_view srcPath, std::string_view dstPath);
bool CreateDirectory(std::string_view newDir);
}