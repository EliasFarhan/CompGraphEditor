#include "engine/filesystem.h"
#include "utils/log.h"
#include <fmt/format.h>
#include <fstream>
#include <filesystem>

#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#endif

namespace fs = std::filesystem;


namespace core
{

FileBuffer::~FileBuffer()
{
    if(data != nullptr)
    {
        std::free(data);
        data = nullptr;
        size = 0;
    }
}

FileBuffer DefaultFilesystem::LoadFile(const Path &path) const
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
    const std::ifstream file(path.c_str(), std::ifstream::binary);
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

bool DefaultFilesystem::FileExists(const Path &path) const
{
    return fs::exists(path.c_str());
}

bool DefaultFilesystem::IsRegularFile(const Path &path) const
{
    return fs::is_regular_file(path.c_str());
}

bool DefaultFilesystem::IsDirectory(const Path &path) const
{
    return fs::is_directory(path.c_str());
}
void DefaultFilesystem::WriteString(const Path &path, std::string_view content) const
{
    std::ofstream outFile(path.c_str(), std::ofstream::binary);
    outFile << content;
}

bool IOSystem::Exists(const char* pFile) const
{
    const auto& filesystem = FilesystemLocator::get();
    return filesystem.FileExists(Path(pFile));
}

char IOSystem::getOsSeparator() const
{
    return '/';
}

Assimp::IOStream* IOSystem::Open(const char* pFile, const char* pMode)
{
    const auto& filesystem = FilesystemLocator::get();

    return new IOStream(filesystem.LoadFile(Path(pFile)));
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
