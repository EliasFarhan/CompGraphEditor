#pragma once

#include <string>
#include <vector>
#include "resource.h"

namespace gpr5300
{


class ResourceManager
{
public:

    void CheckDataFolder();
    ResourceId FindResourceByPath(std::string_view path) const;
    static constexpr std::string_view dataFolder = "data/";
    void RegisterResourceChange(ResourceChangeInterface* resourceChange);
    void AddResource(std::string_view path);

private:
    static ResourceId GenerateResourceId();

    std::vector<Resource> resources_;
    std::vector<ResourceChangeInterface*> resourceChangeInterfaces_;
    void RemoveResource(const Resource& resource);
    void UpdateExistingResource(const Resource& resource);
};


}