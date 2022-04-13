#pragma once

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

};

class NullFilesystem final : public FilesystemInterface
{

};

class DefaultFilesystem final : public FilesystemInterface
{

};

class PhysFilesystem final : public FilesystemInterface
{
public:

private:
};

} // namespace gpr5300
