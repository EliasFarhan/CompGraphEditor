
#include "editor_filesystem.h"
#include <format>
#include "utils/log.h"
#include "engine/filesystem.h"

namespace novus::editor
{

std::string GetFileExtension(std::string_view path)
{
    const fs::path p = path.data();
    return p.extension().string();
}

fs::file_time_type GetLastTimeWrite(std::string_view path)
{
    const fs::path p = path.data();
    return fs::exists(p)?last_write_time(p): fs::file_time_type{};
}
std::string GetFilename(std::string_view path, bool withExtension)
{
    const fs::path p = path;
    return withExtension?p.filename().string() : p.stem().string();
}
bool CopyFileFromTo(std::string_view srcPath, std::string_view dstPath, bool forceOverwrite)
{
    try
    {
        fs::copy(srcPath.data(), dstPath.data(), forceOverwrite?fs::copy_options::overwrite_existing:fs::copy_options::skip_existing);
    }
    catch (fs::filesystem_error& e)
    {
        LogError(std::format("Could not copy from {} to {}\n{}", srcPath.data(), dstPath.data(), e.what()));
        return false;
    }
    return true;
}
bool CreateNewDirectory(std::string_view newDir)
{
    return fs::create_directories(newDir.data());
}

bool RemoveFile(std::string_view path)
{
    const fs::path p = path.data();
    return fs::remove(p);
}

std::string GetFolder(std::string_view path)
{
    const fs::path p = path.data();
    return p.parent_path().string();
}
}
