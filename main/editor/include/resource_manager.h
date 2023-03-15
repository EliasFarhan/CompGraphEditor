#pragma once

#include <string>
#include <vector>
#include "resource.h"

namespace editor
{


class ResourceManager
{
public:

    void CheckDataFolder();
    [[nodiscard]] ResourceId FindResourceByPath(std::string_view path) const;
    [[nodiscard]] const Resource* GetResource(ResourceId resource) const;
    static constexpr std::string_view dataFolder = "data/";
    void RegisterResourceChange(ResourceChangeInterface* resourceChange);
    void AddResource(std::string_view path);
    void RemoveResource(std::string_view path);
private:
    static ResourceId GenerateResourceId();

    std::vector<Resource> resources_;
    std::vector<ResourceChangeInterface*> resourceChangeInterfaces_;
    void RemoveResource(const Resource& resource);
    void UpdateExistingResource(const Resource& resource);
};


}