
#include "editor_filesystem.h"
#include <fmt/format.h>
#include "utils/log.h"

namespace editor
{

std::string GetFileExtension(std::string_view path)
{
    const fs::path p = path;
    return p.extension().string();
}

fs::file_time_type GetLastTimeWrite(std::string_view path)
{
    const fs::path p = path;
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
        fs::copy(srcPath, dstPath, forceOverwrite?fs::copy_options::overwrite_existing:fs::copy_options::skip_existing);
    }
    catch (fs::filesystem_error& e)
    {
        LogError(fmt::format("Could not copy from {} to {}\n{}", srcPath, dstPath, e.what()));
        return false;
    }
    return true;
}
bool CreateNewDirectory(std::string_view newDir)
{
    return fs::create_directories(newDir);
}

bool RemoveFile(std::string_view path)
{
    const fs::path p = path;
    return fs::remove(p);
}

std::string GetFolder(std::string_view path)
{
    const fs::path p = path;
    return p.parent_path().string();
}
}
