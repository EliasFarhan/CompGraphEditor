#include <unordered_set>
#include <functional>
#include <algorithm>
#include "resource_manager.h"

namespace gpr5300
{

void ResourceManager::CheckDataFolder()
{
    auto& filesystem = FilesystemLocator::get();
    // Remove deleted file
    std::unordered_set<std::string_view> currentResources;
    auto fileIt = resources_.begin();
    while(fileIt != resources_.end())
    {
        if(!filesystem.IsRegularFile(fileIt->path))
        {
            RemoveResource(*fileIt);
            fileIt = resources_.erase(fileIt);
        }
        else
        {
            fileIt++;
            currentResources.emplace(fileIt->path);
        }
    }
    std::function<void(std::string_view)> recursiveIterateFile = [this, &filesystem, &recursiveIterateFile, &currentResources]
        (std::string_view directoryPath)
    {
        fs::path dir = directoryPath;
        for(const auto& entry : fs::directory_iterator(dir))
        {
            auto path = entry.path();
            if(filesystem.IsDirectory(path.c_str()))
            {
                recursiveIterateFile(path.c_str());
            }
            else
            {
                if(!currentResources.contains(path.c_str()))
                {
                    AddResource(path.c_str());
                }
                else
                {
                    auto it = std::ranges::find_if(resources_,[&path](const Resource& resource)
                    {
                        return resource.path == path;
                    });
                    if(it->lastTimeWrite < GetLastTimeWrite(path.c_str()))
                    {
                        UpdateResource(*it);
                    }
                }
            }
        }
    };

    recursiveIterateFile(dataFolder);
}
ResourceId ResourceManager::GenerateResourceId()
{
    static ResourceId resourceId = 1;
    return resourceId++;
}
void ResourceManager::RemoveResource(const Resource &resource)
{
    for(auto* resourceChange : resourceChangeInterfaces_)
    {
        resourceChange->RemoveResource(resource);
    }
}
void ResourceManager::AddResource(std::string_view path)
{
    Resource newResource{};
    newResource.path = path;
    newResource.extension = GetFileExtension(path);
    newResource.lastTimeWrite = GetLastTimeWrite(path);
    newResource.resourceId = GenerateResourceId();

    resources_.push_back(newResource);
    for(auto* resourceChange : resourceChangeInterfaces_)
    {
        resourceChange->AddResource(newResource);
    }

}
void ResourceManager::UpdateResource(const Resource &resource)
{
    for(auto* resourceChange : resourceChangeInterfaces_)
    {
        resourceChange->UpdateResource(resource);
    }
}
void ResourceManager::RegisterResourceChange(ResourceChangeInterface *resourceChange)
{
    resourceChangeInterfaces_.push_back(resourceChange);
}
}