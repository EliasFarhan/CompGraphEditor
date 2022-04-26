
#include "editor_filesystem.h"
#include <fmt/format.h>
#include "utils/log.h"

namespace gpr5300
{

std::string GetFileExtension(std::string_view path)
{
    fs::path p = path;
    return p.extension().string();
}

fs::file_time_type GetLastTimeWrite(std::string_view path)
{
    fs::path p = path;
    return last_write_time(p);
}
std::string GetFilename(std::string_view path, bool withExtension)
{
    fs::path p = path;
    return withExtension?p.filename().string() : p.stem().string();
}
bool CopyFileFromTo(std::string_view srcPath, std::string_view dstPath)
{
    try
    {
        fs::copy(srcPath, dstPath);
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
}