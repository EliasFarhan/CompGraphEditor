#include "engine/filesystem.h"
#include "utils/log.h"
#include <format>
#include <fstream>
#include <filesystem>

#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#endif



namespace core
{

namespace
{
DefaultFilesystem defaultFilesystem;
FilesystemInterface* filesystem = &defaultFilesystem;
}
namespace fs = std::filesystem;

FileBuffer::~FileBuffer()
{
    if(data != nullptr)
    {
        std::free(data);
        data = nullptr;
        size = 0;
    }
}

FileBuffer DefaultFilesystem::LoadFile(std::string_view path) const
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    // allocate memory to contain file data
    FileBuffer bufferFile;
    if(!FileExists(path))
    {
        return bufferFile;
    }
    const std::ifstream file(path.data(), std::ifstream::binary);
    // get pointer to associated buffer object
    std::filebuf* pbuf = file.rdbuf();

    // get file size using buffer's members
    const std::size_t size = pbuf->pubseekoff(0, std::ifstream::end, file.in);
    pbuf->pubseekpos(0, std::ifstream::in);

    bufferFile.data = static_cast<unsigned char*>(std::malloc(size + 1));
    

    // get file data
    pbuf->sgetn(reinterpret_cast<char*>(bufferFile.data), size);
    bufferFile.data[size] = 0;
    bufferFile.size = size;
    return bufferFile;
}

bool DefaultFilesystem::FileExists(std::string_view path) const
{
    return fs::exists(path.data());
}

bool DefaultFilesystem::IsRegularFile(std::string_view path) const
{
    return fs::is_regular_file(path.data());
}

bool DefaultFilesystem::IsDirectory(std::string_view path) const
{
    return fs::is_directory(path.data());
}
void DefaultFilesystem::WriteString(std::string_view path, std::string_view content) const
{
    std::ofstream outFile(path.data(), std::ofstream::binary);
    outFile << content;
}
void SetFileSystem(FilesystemInterface *fs)
{
    filesystem = fs;
}

FileBuffer LoadFile(std::string_view path)
{
    return filesystem->LoadFile(path);
}
bool FileExists(std::string_view path)
{
    return filesystem->FileExists(path);
}
bool IsRegularFile(std::string_view path)
{
    return filesystem->IsRegularFile(path);
}
bool IsDirectory(std::string_view path)
{
    return filesystem->IsDirectory(path);
}
void WriteString(std::string_view path, std::string_view content)
{
    filesystem->WriteString(path, content);
}

bool IOSystem::Exists(const char* pFile) const
{
    return filesystem->FileExists(pFile);
}

char IOSystem::getOsSeparator() const
{
    return '/';
}

Assimp::IOStream* IOSystem::Open(const char* pFile, const char* pMode)
{
    return new IOStream(filesystem->LoadFile(pFile));
}

void IOSystem::Close(Assimp::IOStream* pFile)
{
    delete pFile;
}

IOStream::IOStream(FileBuffer&& bufferFile): bufferFile_(std::move(bufferFile))
{
}

size_t IOStream::Read(void* pvBuffer, size_t pSize, size_t pCount)
{
    const auto totalReadSize = pSize * pCount;
    auto readSize = totalReadSize;
    if(totalReadSize > bufferFile_.size - cursorIndex_)
    {
        readSize = bufferFile_.size - cursorIndex_;
    }
    std::memcpy(pvBuffer, bufferFile_.data + cursorIndex_, readSize);
    return readSize;
}

size_t IOStream::Write(const void* pvBuffer, size_t pSize, size_t pCount)
{
    return {};
}

aiReturn IOStream::Seek(size_t pOffset, aiOrigin pOrigin)
{
    switch (pOrigin)
    {
    case aiOrigin_SET: 
        cursorIndex_ = pOffset;
        break;
    case aiOrigin_CUR: 
        cursorIndex_ += pOffset;
        break;
    case aiOrigin_END: 
        cursorIndex_ = bufferFile_.size - pOffset;
        break;
    default: ;
    }
    return aiReturn_SUCCESS;
}

size_t IOStream::Tell() const
{
    return cursorIndex_;
}

size_t IOStream::FileSize() const
{
    return bufferFile_.size;
}

void IOStream::Flush()
{
}



}
