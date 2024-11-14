#pragma once

#include <filesystem>
#include "engine/filesystem.h"

namespace fs = std::filesystem;

namespace editor
{
std::string GetFileExtension(std::string_view path);
std::string GetFilename(std::string_view path, bool withExtension=true);
fs::file_time_type GetLastTimeWrite(std::string_view pathpath);
bool CopyFileFromTo(std::string_view pathsrcPath, std::string_view pathdstPath, bool forceOverwrite=false);
bool CreateNewDirectory(std::string_view pathnewDir);
bool RemoveFile(std::string_view pathpath);
std::string GetFolder(std::string_view pathpath);
}