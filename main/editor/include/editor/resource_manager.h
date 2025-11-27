#pragma once

#include <span>
#include <string>
#include <vector>
#include "resource.h"

#include "engine/filesystem.h"


namespace novus::editor
{


class ResourceManager
{
public:

    void CheckDataFolder(std::span<const std::string> paths);
    [[nodiscard]] ResourceId FindResourceByPath(std::string_view path) const;
    [[nodiscard]] const Resource* GetResource(ResourceId resource) const;
    static constexpr std::string_view dataFolder = "data/";
    void RegisterResourceChange(ResourceChangeInterface* resourceChange);
    void AddResource(std::string_view path);
    void RemoveResource(std::string_view path, bool deleteFile=true);
    void UpdateExistingResource(const Resource& resource);
    void Clear();
private:
    static ResourceId GenerateResourceId();

    std::vector<Resource> resources_;
    std::vector<ResourceChangeInterface*> resourceChangeInterfaces_;
    void RemoveResource(const Resource& resource);
};


}