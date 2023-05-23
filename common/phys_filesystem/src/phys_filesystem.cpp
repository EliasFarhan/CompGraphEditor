#include "phys_filesystem.h"

#include <physfs.h>
#include <fmt/format.h>

#include "utils/log.h"
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

namespace core
{
    PhysFilesystem::PhysFilesystem(const char* argv0) : argv0_(argv0)
    {
    }

    void PhysFilesystem::Begin()
    {
        if (!PHYSFS_init(argv0_.c_str()))
        {
            LogError(fmt::format("PhysFS init failed {}", static_cast<int>(PHYSFS_getLastErrorCode())));
        }
    }

    void PhysFilesystem::End()
    {
        if (!PHYSFS_deinit())
        {
            LogError(fmt::format("PhysFS deinit failed: {}", static_cast<int>(PHYSFS_getLastErrorCode())));
        }
    }

    void PhysFilesystem::AddMount(std::string_view dir, std::string_view mountPoint, int append) const
    {
        if (!PHYSFS_mount(dir.data(), mountPoint.data(), append))
        {
            LogError(fmt::format(
                "PhysFS could not mount: {} at mount point: {}\n Log: {}",
                dir, mountPoint, static_cast<int>(PHYSFS_getLastErrorCode())));
        }
    }

    core::BufferFile PhysFilesystem::LoadFile(std::string_view path) const
    {
        std::string genericPath = path.data();
        std::ranges::replace(genericPath, '\\', '/');
        core::BufferFile newFile;
        if (!FileExists(genericPath))
        {
            LogError(fmt::format("File does not exist: {}", genericPath));
            return newFile;
        }
        auto* file = PHYSFS_openRead(genericPath.data());
        newFile.length = PHYSFS_fileLength(file);
        newFile.data = static_cast<unsigned char*>(
            std::malloc(newFile.length + 1));
        newFile.data[newFile.length] = 0;
        if (PHYSFS_readBytes(file, newFile.data, newFile.length) == -1)
        {
            LogError(fmt::format("Physfs could not read file: {}\nLog: {}",
                genericPath, static_cast<int>(PHYSFS_getLastErrorCode())));
            PHYSFS_close(file);
            return {};
        }
        PHYSFS_close(file);
        return newFile;
    }

    bool PhysFilesystem::FileExists(std::string_view path) const
    {
        std::string genericPath = path.data();
        std::ranges::replace(genericPath, '\\', '/');
        return PHYSFS_exists(genericPath.data());
    }

    bool PhysFilesystem::IsRegularFile(std::string_view path) const
    {
        std::string genericPath = path.data();
        std::ranges::replace(genericPath, '\\', '/');
        PHYSFS_Stat stat;
        if (PHYSFS_stat(genericPath.data(), &stat))
        {
            LogError(fmt::format(
                "PhysFS could not get stat of file: {}\nLog: {}",
                genericPath, static_cast<int>(PHYSFS_getLastErrorCode())));
            return false;
        }
        return stat.filetype == PHYSFS_FILETYPE_REGULAR;
    }

    bool PhysFilesystem::IsDirectory(std::string_view path) const
    {
        PHYSFS_Stat stat;
        if (PHYSFS_stat(path.data(), &stat))
        {
            LogError(fmt::format(
                "PhysFS could not get stat of file: {}\nLog: {}",
                path, static_cast<int>(PHYSFS_getLastErrorCode())));
            return false;
        }
        return stat.filetype == PHYSFS_FILETYPE_DIRECTORY;
    }
    void PhysFilesystem::WriteString(std::string_view path, std::string_view content) const
    {

    }
} // namespace gpr5300
