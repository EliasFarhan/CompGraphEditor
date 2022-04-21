#include "engine/filesystem.h"

#include <fstream>
#include <filesystem>


namespace fs = std::filesystem;

namespace gpr5300
{

BufferFile::~BufferFile()
{
    if(data != nullptr)
    {
        std::free(data);
        data = nullptr;
        length = 0;
    }
}

BufferFile DefaultFilesystem::LoadFile(std::string_view path) const
{
    // allocate memory to contain file data
    BufferFile bufferFile;

    const std::ifstream file(path.data(), std::ifstream::binary);
    // get pointer to associated buffer object
    std::filebuf* pbuf = file.rdbuf();

    // get file size using buffer's members
    const std::size_t size = pbuf->pubseekoff(0, file.end, file.in);
    pbuf->pubseekpos(0, file.in);

    bufferFile.data = static_cast<unsigned char*>(std::malloc(size + 1));
    

    // get file data
    pbuf->sgetn(reinterpret_cast<char*>(bufferFile.data), size);
    bufferFile.data[size] = 0;
    bufferFile.length = size;
    return bufferFile;
}

bool DefaultFilesystem::FileExists(std::string_view path) const
{
    return fs::exists(path);
}

bool DefaultFilesystem::IsRegularFile(std::string_view path) const
{
    return fs::is_regular_file(path);
}

bool DefaultFilesystem::IsDirectory(std::string_view path) const
{
    return fs::is_directory(path);
}
void DefaultFilesystem::WriteString(std::string_view path, std::string_view content) const
{
    std::ofstream outFile(path.data());
    outFile << content;
}

BufferFile PhysFilesystem::LoadFile(std::string_view path) const
{
    return {};
}

bool PhysFilesystem::FileExists(std::string_view path) const
{
    return false;
}

bool PhysFilesystem::IsRegularFile(std::string_view path) const
{
    return false;
}

bool PhysFilesystem::IsDirectory(std::string_view path) const
{
    return false;
}
void PhysFilesystem::WriteString(std::string_view path, std::string_view content) const
{

}
}
