#pragma once

#include "utils/locator.h"

#include <assimp/IOSystem.hpp>
#include <assimp/IOStream.hpp>
#include <fmt/format.h>

#include <cassert>
#include <string_view>
#include <array>



namespace core
{

class Path
{
public:
    constexpr Path() = default;
    explicit constexpr Path(const std::string& path)
    {
        std::char_traits<char>::copy(path_.data(), path.c_str(), std::min(path_.size(), path.size()));
    }
    explicit constexpr Path(std::string_view path)
    {
        std::char_traits<char>::copy(path_.data(), path.data(), std::min(path_.size(), path.size()));
    }
    constexpr Path(const char* path)
    {
        std::char_traits<char>::copy(path_.data(), path, std::min(path_.size(), std::char_traits<char>::length(path)));
    }
    [[nodiscard]] constexpr const char* c_str() const
    {
        return path_.data();
    }
    constexpr operator std::string_view() const
    {
        return { path_.data(), size() };
    }
    constexpr explicit operator std::string() const
    {
        return path_.data();
    }
    constexpr bool operator==(const Path& other) const
    {
        return std::char_traits<char>::compare(path_.data(), other.path_.data(), size()) == 0;
    }
    [[nodiscard]] constexpr std::size_t size() const
    {
        return std::char_traits<char>::length(path_.data());
    }

    [[nodiscard]] constexpr bool empty() const
    {
        return size() == 0;
    }
    [[nodiscard]] constexpr Path operator+(const Path& other) const
    {
        Path result{};
        std::char_traits<char>::copy(result.path_.data(), path_.data(), path_.size());
        const auto oldSize = result.size();
        std::char_traits<char>::copy(result.path_.data()+result.size(), other.path_.data(), other.path_.size());
        *(result.path_.data()+oldSize+other.path_.size()) = 0;
        return result;
    }

    constexpr Path& operator+=(const Path& other)
    {
        const auto oldSize = size();
        std::char_traits<char>::copy(path_.data()+size(), other.path_.data(), other.path_.size());
        *(path_.data()+oldSize+other.path_.size()) = 0;
        return *this;
    }


    [[nodiscard]] bool contains(const Path& path) const
    {
        return std::strstr(path_.data(), path.path_.data()) != nullptr;
    }

    using iterator = char*;
    using const_iterator = const char*;


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
    [[nodiscard]] virtual FileBuffer LoadFile(const Path &path) const = 0;
    [[nodiscard]] virtual bool FileExists(const Path &path) const = 0;
    [[nodiscard]] virtual bool IsRegularFile(const Path &path) const = 0;
    [[nodiscard]] virtual bool IsDirectory(const Path &path) const = 0;
    virtual void WriteString(const Path &path, std::string_view content) const = 0;
};

class NullFilesystem final : public FilesystemInterface
{
public:
    [[nodiscard]] FileBuffer LoadFile(const Path &path) const override
    {
        assert(false);
        return {};
    }
    [[nodiscard]] bool FileExists(const Path &path) const override
    {
        assert(false);
        return false;
    }
    [[nodiscard]] bool IsRegularFile(const Path &path) const override
    {
        assert(false);
        return false;
    }
    [[nodiscard]] bool IsDirectory(const Path &path) const override
    {
        assert(false);
        return false;
    }
    void WriteString(const Path &path, std::string_view content) const override
    {
        assert(false);
    }
};

class DefaultFilesystem final : public FilesystemInterface
{
public:
    [[nodiscard]] FileBuffer LoadFile(const Path &path) const override;
    [[nodiscard]] bool FileExists(const Path &path) const override;
    [[nodiscard]] bool IsRegularFile(const Path &path) const override;
    [[nodiscard]] bool IsDirectory(const Path &path) const override;
    void WriteString(const Path &path, std::string_view content) const override;
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

template<>
struct fmt::formatter<core::Path>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(core::Path const& number, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "{}", number.c_str());
    }
};