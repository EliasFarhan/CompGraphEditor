#pragma once

#include <cassert>

#include "utils/locator.h"
#include <string_view>

namespace gpr5300
{
    
class BufferFile
{
public:
    BufferFile();
    BufferFile(std::string_view path);
    ~BufferFile();
    BufferFile(const BufferFile&) = delete;
    BufferFile& operator=(const BufferFile&) = delete;
    BufferFile(BufferFile&& other) noexcept = default;
    BufferFile& operator=(BufferFile&&) noexcept = default;


    unsigned char* data;
};

class FilesystemInterface
{
public:
    virtual ~FilesystemInterface() = default;
    [[nodiscard]] virtual BufferFile LoadFile(std::string_view path) const = 0;
    [[nodiscard]] virtual bool FileExists(std::string_view) const = 0;
    [[nodiscard]] virtual bool IsRegularFile(std::string_view) const = 0;
    [[nodiscard]] virtual bool IsDirectory(std::string_view) const = 0;
};

class NullFilesystem final : public FilesystemInterface
{
public:
    [[nodiscard]] BufferFile LoadFile(std::string_view path) const override
    {
        assert(false);
        return {};
    }
    [[nodiscard]] bool FileExists(std::string_view) const override
    {
        assert(false);
        return false;
    }
    [[nodiscard]] bool IsRegularFile(std::string_view) const override
    {
        assert(false);
        return false;
    }
    [[nodiscard]] bool IsDirectory(std::string_view) const override
    {
        assert(false);
        return false;
    }
};

class DefaultFilesystem final : public FilesystemInterface
{
public:
    [[nodiscard]] BufferFile LoadFile(std::string_view path) const override;
    [[nodiscard]] bool FileExists(std::string_view path) const override;
    [[nodiscard]] bool IsRegularFile(std::string_view path) const override;
    [[nodiscard]] bool IsDirectory(std::string_view path) const override;
};

class PhysFilesystem final : public FilesystemInterface
{
public:
    [[nodiscard]] BufferFile LoadFile(std::string_view path) const override;
    [[nodiscard]] bool FileExists(std::string_view path) const override;
    [[nodiscard]] bool IsRegularFile(std::string_view path) const override;
    [[nodiscard]] bool IsDirectory(std::string_view path) const override;
    
};

using FilesystemLocator = Locator<FilesystemInterface, NullFilesystem>;

} // namespace gpr5300
