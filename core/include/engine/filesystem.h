#pragma once

#include "utils/locator.h"

#include <assimp/IOSystem.hpp>
#include <assimp/IOStream.hpp>

#include <cassert>
#include <string_view>
#include <array>



namespace core
{

class Path
{
public:
    Path() = default;
    Path(const std::string& path);
    Path(std::string_view path);
    Path(const char* path);
    [[nodiscard]] const char* c_str() const;
    explicit operator std::string_view() const;
    bool operator==(const Path& other) const;

    using iterator = char*;
    using const_iterator = const char*;

    // Class methods
    std::size_t size() const;
    const_iterator cbegin() const { return path_.data(); }
    const_iterator cend() const { return path_.data() + size(); }
    const_iterator begin() const { return cbegin(); }
    const_iterator end() const { return cend(); }
    iterator begin() { return path_.data(); }
    iterator end() { return path_.data() + size(); }

    static constexpr std::size_t MAX_PATH_LENGTH = 48;
private:
    std::array<char, MAX_PATH_LENGTH> path_{};
};
    
class FileBuffer
{
public:
    FileBuffer() = default;
    ~FileBuffer();
    FileBuffer(const FileBuffer&) = delete;
    FileBuffer& operator=(const FileBuffer&) = delete;
    FileBuffer(FileBuffer&& other) noexcept
    {
        std::swap(data, other.data);
        std::swap(size, other.size);
    }
    FileBuffer& operator=(FileBuffer&& other) noexcept
    {
        std::swap(data, other.data);
        std::swap(size, other.size);
        return *this;
    }

    unsigned char* data = nullptr;
    std::size_t size = 0;
};

class FilesystemInterface
{
public:
    virtual ~FilesystemInterface() = default;
    [[nodiscard]] virtual FileBuffer LoadFile(Path path) const = 0;
    [[nodiscard]] virtual bool FileExists(Path path) const = 0;
    [[nodiscard]] virtual bool IsRegularFile(Path path) const = 0;
    [[nodiscard]] virtual bool IsDirectory(Path path) const = 0;
    virtual void WriteString(Path path, std::string_view content) const = 0;
};

class NullFilesystem final : public FilesystemInterface
{
public:
    [[nodiscard]] FileBuffer LoadFile(Path path) const override
    {
        assert(false);
        return {};
    }
    [[nodiscard]] bool FileExists(Path path) const override
    {
        assert(false);
        return false;
    }
    [[nodiscard]] bool IsRegularFile(Path path) const override
    {
        assert(false);
        return false;
    }
    [[nodiscard]] bool IsDirectory(Path path) const override
    {
        assert(false);
        return false;
    }
    void WriteString(Path path, std::string_view content) const override
    {
        assert(false);
    }
};

class DefaultFilesystem final : public FilesystemInterface
{
public:
    [[nodiscard]] FileBuffer LoadFile(Path path) const override;
    [[nodiscard]] bool FileExists(Path path) const override;
    [[nodiscard]] bool IsRegularFile(Path path) const override;
    [[nodiscard]] bool IsDirectory(Path path) const override;
    void WriteString(Path path, std::string_view content) const override;
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
    IOStream(FileBuffer&& bufferFile);
    size_t Read(void* pvBuffer, size_t pSize, size_t pCount) override;
    size_t Write(const void* pvBuffer, size_t pSize, size_t pCount) override;
    aiReturn Seek(size_t pOffset, aiOrigin pOrigin) override;
    size_t Tell() const override;
    size_t FileSize() const override;
    void Flush() override;
private:
    FileBuffer bufferFile_{};
    std::size_t cursorIndex_ = 0;
};

} // namespace core

template<>
struct std::hash<core::Path>
{
    std::size_t operator()(core::Path const& s) const noexcept
    {
        return std::hash<std::string_view>{}((std::string_view)s);
    }
};