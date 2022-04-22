#include <unordered_set>
#include <functional>
#include <algorithm>
#include "editor_resource.h"

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
            if(filesystem.IsDirectory(dir.c_str()))
            {
                recursiveIterateFile(dir.c_str());
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
void ResourceManager::RemoveResource(const Resource &Resource)
{
    //TODO call the editor to remove an editor resource
}
void ResourceManager::AddResource(std::string_view path)
{
    Resource newResource{};
    newResource.path = path;
    newResource.extension = GetFileExtension(path);
    newResource.lastTimeWrite = GetLastTimeWrite(path);
    newResource.resourceId = GenerateResourceId();

    resources_.push_back(newResource);
    //TODO call the editor to add a new editor resource

}
void ResourceManager::UpdateResource(const Resource &Resource)
{
    //TODO call the editor to update an editor resource
}
}