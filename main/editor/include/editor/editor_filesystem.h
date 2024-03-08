#pragma once

#include <filesystem>
#include "engine/filesystem.h"

namespace fs = std::filesystem;

namespace editor
{
std::string GetFileExtension(core::Path path);
std::string GetFilename(std::string_view path, bool withExtension=true);
fs::file_time_type GetLastTimeWrite(const core::Path &path);
bool CopyFileFromTo(const core::Path &srcPath, const core::Path &dstPath, bool forceOverwrite=false);
bool CreateNewDirectory(const core::Path &newDir);
bool RemoveFile(const core::Path &path);
core::Path GetFolder(const core::Path &path);
}