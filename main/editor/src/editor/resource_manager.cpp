#include <unordered_set>
#include <functional>
#include <algorithm>
#include "resource_manager.h"
#include "engine/filesystem.h"

namespace editor
{

void ResourceManager::CheckDataFolder(const PbRepeatField<std::string>& paths)
{
    auto& filesystem = core::FilesystemLocator::get();
    // Remove deleted file
    for(auto& path : paths)
    {
        if(!filesystem.IsRegularFile(path))
        {
            RemoveResource(path);
        }
        else
        {
            auto it = std::ranges::find_if(resources_, [&path](const Resource& resource)
                {
                    return fs::equivalent(resource.path, path);
                });
            if(it != resources_.end())
            {
                if (it->lastTimeWrite < GetLastTimeWrite(path))
                {
                    UpdateExistingResource(*it);
                }
            }
            else
            {
                AddResource(path);
            }
        }
    }
}

ResourceId ResourceManager::FindResourceByPath(std::string_view path) const
{
    if (path.empty())
        return INVALID_RESOURCE_ID;
    const auto& fileSystem = core::FilesystemLocator::get();
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
    static unsigned resourceId = { 1 };
    return { resourceId++ };
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

void ResourceManager::RemoveResource(std::string_view path, bool deleteFile)
{
    const auto resourceId = FindResourceByPath(path);
    if(resourceId != INVALID_RESOURCE_ID)
    {
        const auto it = std::ranges::find_if(resources_, [resourceId](const auto& resource)
            {
                return resourceId == resource.resourceId;
            });
        if (deleteFile)
        {
            RemoveFile(path);
        }
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

void ResourceManager::Clear()
{
    resources_.clear();
}

void ResourceManager::RegisterResourceChange(ResourceChangeInterface *resourceChange)
{
    resourceChangeInterfaces_.push_back(resourceChange);
}
}