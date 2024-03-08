
#include "editor_filesystem.h"
#include <fmt/format.h>
#include "utils/log.h"
#include "engine/filesystem.h"

namespace editor
{

std::string GetFileExtension(core::Path path)
{
    const fs::path p = path.c_str();
    return p.extension().string();
}

fs::file_time_type GetLastTimeWrite(const core::Path &path)
{
    const fs::path p = path.c_str();
    return fs::exists(p)?last_write_time(p): fs::file_time_type{};
}
std::string GetFilename(std::string_view path, bool withExtension)
{
    const fs::path p = path;
    return withExtension?p.filename().string() : p.stem().string();
}
bool CopyFileFromTo(const core::Path &srcPath, const core::Path &dstPath, bool forceOverwrite)
{
    try
    {
        fs::copy(srcPath.c_str(), dstPath.c_str(), forceOverwrite?fs::copy_options::overwrite_existing:fs::copy_options::skip_existing);
    }
    catch (fs::filesystem_error& e)
    {
        LogError(fmt::format("Could not copy from {} to {}\n{}", srcPath, dstPath, e.what()));
        return false;
    }
    return true;
}
bool CreateNewDirectory(const core::Path &newDir)
{
    return fs::create_directories(newDir.c_str());
}

bool RemoveFile(const core::Path &path)
{
    const fs::path p = path.c_str();
    return fs::remove(p);
}

core::Path GetFolder(const core::Path &path)
{
    const fs::path p = path.c_str();
    return core::Path{p.parent_path().string()};
}
}
