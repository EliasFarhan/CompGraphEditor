#include <unordered_set>
#include <functional>
#include <algorithm>
#include "resource_manager.h"
#include "engine/filesystem.h"

namespace gpr5300
{

void ResourceManager::CheckDataFolder()
{
    auto& filesystem = FilesystemLocator::get();
    // Remove deleted file
    std::vector <fs::path> currentResources;
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
            currentResources.push_back(fileIt->path);
            ++fileIt;
        }
    }
    std::function<void(std::string_view)> recursiveIterateFile = [this, &filesystem, &recursiveIterateFile, &currentResources]
        (std::string_view directoryPath)
    {
        const fs::path dir = directoryPath;
        for(const auto& entry : fs::directory_iterator(dir))
        {
            auto path = entry.path();
            if(filesystem.IsDirectory(path.string()))
            {
                recursiveIterateFile(path.string());
            }
            else
            {
                bool newResource = true;
                for(auto& currentResource : currentResources)
                {
                    if(fs::equivalent(currentResource, path))
                    {
                        auto it = std::ranges::find_if(resources_, [&path](const Resource& resource)
                            {
                                return fs::equivalent(resource.path,path);
                            });
                        if (it->lastTimeWrite < GetLastTimeWrite(path.string()))
                        {
                            UpdateExistingResource(*it);
                        }
                        newResource = false;
                        break;
                    }
                }
                if(newResource)
                {
                    AddResource(path.string());
                }
            }
        }
    };

    recursiveIterateFile(dataFolder);
}

ResourceId ResourceManager::FindResourceByPath(std::string_view path) const
{
    if (path.empty())
        return INVALID_RESOURCE_ID;
    const auto& fileSystem = FilesystemLocator::get();
    if(!fileSystem.FileExists(path))
    {
        return INVALID_RESOURCE_ID;
    }
    const auto it = std::ranges::find_if(resources_, [path](const auto& resource)
    {
        return fs::equivalent(path,resource.path);
    });
    if(it != resources_.end())
    {
        return it->resourceId;
    }
    return INVALID_RESOURCE_ID;
}

const Resource* ResourceManager::GetResource(ResourceId resourceId) const
{
    const auto it = std::ranges::find_if(resources_, [resourceId](const auto& resource)
    {
        return resourceId == resource.resourceId;
    });
    if(it != resources_.end())
    {
        return &*it;
    }
    return nullptr;
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

void ResourceManager::RemoveResource(std::string_view path)
{
    const auto resourceId = FindResourceByPath(path);
    if(resourceId != INVALID_RESOURCE_ID)
    {
        const auto it = std::ranges::find_if(resources_, [resourceId](const auto& resource)
            {
                return resourceId == resource.resourceId;
            });
        RemoveFile(path);
        RemoveResource(*it);
        resources_.erase(it);
    }
}

void ResourceManager::UpdateExistingResource(const Resource &resource)
{
    for(auto* resourceChange : resourceChangeInterfaces_)
    {
        resourceChange->UpdateExistingResource(resource);
    }
}
void ResourceManager::RegisterResourceChange(ResourceChangeInterface *resourceChange)
{
    resourceChangeInterfaces_.push_back(resourceChange);
}
}