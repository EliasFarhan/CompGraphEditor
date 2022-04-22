#pragma once

#include <string>
#include <vector>
#include "engine/filesystem.h"

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
class ResourceManager
{
public:

    void CheckDataFolder();
    static constexpr std::string_view dataFolder = "data/";
private:
    static ResourceId GenerateResourceId();

    std::vector<Resource> resources_;

    void RemoveResource(const Resource& Resource);
    void UpdateResource(const Resource& Resource);
    void AddResource(std::string_view path);
};
}