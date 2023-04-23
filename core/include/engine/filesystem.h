#pragma once


#include <assimp/IOSystem.hpp>
#include <assimp/IOStream.hpp>

#include "utils/locator.h"
#include <cassert>
#include <string_view>

#include "renderer/buffer.h"


namespace core
{
    
class BufferFile
{
public:
    BufferFile() = default;
    ~BufferFile();
    BufferFile(const BufferFile&) = delete;
    BufferFile& operator=(const BufferFile&) = delete;
    BufferFile(BufferFile&& other) noexcept
    {
        std::swap(data, other.data);
        std::swap(length, other.length);
    }
    BufferFile& operator=(BufferFile&& other) noexcept
    {
        std::swap(data, other.data);
        std::swap(length, other.length);
        return *this;
    }


    unsigned char* data = nullptr;
    std::size_t length = 0;
};

class FilesystemInterface
{
public:
    virtual ~FilesystemInterface() = default;
    [[nodiscard]] virtual BufferFile LoadFile(std::string_view path) const = 0;
    [[nodiscard]] virtual bool FileExists(std::string_view) const = 0;
    [[nodiscard]] virtual bool IsRegularFile(std::string_view) const = 0;
    [[nodiscard]] virtual bool IsDirectory(std::string_view) const = 0;
    virtual void WriteString(std::string_view path, std::string_view content) const = 0;
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
    void WriteString(std::string_view path, std::string_view content) const override
    {
        assert(false);
    }
};

class DefaultFilesystem final : public FilesystemInterface
{
public:
    [[nodiscard]] BufferFile LoadFile(std::string_view path) const override;
    [[nodiscard]] bool FileExists(std::string_view path) const override;
    [[nodiscard]] bool IsRegularFile(std::string_view path) const override;
    [[nodiscard]] bool IsDirectory(std::string_view path) const override;
    void WriteString(std::string_view path, std::string_view content) const override;
};


using FilesystemLocator = Locator<FilesystemInterface, NullFilesystem>;

/**
 * \brief IOSystem is our own implementation for Assimp IO handling using our own filesystem
 */
class IOSystem : public Assimp::IOSystem
{
public:
    bool Exists(const char* pFile) const override;
    char getOsSeparator() const override;
    Assimp::IOStream* Open(const char* pFile, const char* pMode) override;
    void Close(Assimp::IOStream* pFile) override;
};

class IOStream : public Assimp::IOStream
{
public:
    IOStream(BufferFile&& bufferFile);
    size_t Read(void* pvBuffer, size_t pSize, size_t pCount) override;
    size_t Write(const void* pvBuffer, size_t pSize, size_t pCount) override;
    aiReturn Seek(size_t pOffset, aiOrigin pOrigin) override;
    size_t Tell() const override;
    size_t FileSize() const override;
    void Flush() override;
private:
    BufferFile bufferFile_{};
    std::size_t cursorIndex_ = 0;
};

} // namespace core
