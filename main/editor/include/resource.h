#pragma once

#include "editor_filesystem.h"

namespace gpr5300
{

using ResourceId = std::uint32_t;
constexpr ResourceId INVALID_RESOURCE_ID = 0;
struct Resource
{
    ResourceId resourceId = INVALID_RESOURCE_ID;
    std::string path;
    std::string extension;
    fs::file_time_type lastTimeWrite;
};

class ResourceChangeInterface
{
public:
    virtual void AddResource(const Resource& resource) = 0;
    virtual void RemoveResource(const Resource& resource) = 0;
    virtual void UpdateExistingResource(const Resource& resource) = 0;
};

}