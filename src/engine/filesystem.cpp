#include "engine/filesystem.h"

namespace gpr5300
{
BufferFile DefaultFilesystem::LoadFile(std::string_view path) const
{
    return {};
}

bool DefaultFilesystem::FileExists(std::string_view path) const
{
    return false;
}

bool DefaultFilesystem::IsRegularFile(std::string_view path) const
{
    return false;
}

bool DefaultFilesystem::IsDirectory(std::string_view path) const
{
    return false;
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
}
