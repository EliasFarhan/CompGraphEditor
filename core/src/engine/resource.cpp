#include "engine/resource.h"
#include "utils/job_system.h"
#include "utils/log.h"

#include <fmt/format.h>

#include <filesystem>
namespace fs = std::filesystem;

#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#endif

namespace core
{

ResourceManager* instance = nullptr;

ResourceManager::ResourceManager()
{
    instance = this;
}

void ResourceManager::Begin()
{
    auto* jobSystem = GetJobSystem();
    resourceLoadQueue_ = jobSystem->SetupNewQueue();
}

void ResourceManager::Update(float dt)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    //execute all moving filebuffer tasks
    while(!moveFileBufferJobs_.empty())
    {
        std::unique_ptr<MoveFileBufferJob> job = nullptr;
        {
            std::scoped_lock lock(resourceLoadMutex_);
            job = std::move(moveFileBufferJobs_.front());
            moveFileBufferJobs_.pop();
        }
        job->Execute();
    }
}

void ResourceManager::End()
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    while(!moveFileBufferJobs_.empty())
    {
        moveFileBufferJobs_.pop();
    }
    fileBuffers_.clear();
}

ResourceId ResourceManager::AddResource(std::string_view path)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    if (path.empty())
        return INVALID_RESOURCE_ID;

    const auto& filesystem = core::FilesystemLocator::get();
    if(!filesystem.FileExists(Path(path)))
    {
        LogWarning(fmt::format("Adding unexisting resource: {}", path));
        return INVALID_RESOURCE_ID;
    }

    const auto it = std::ranges::find_if(resources_, [path, &filesystem](const auto& resource)
    {
        if (!filesystem.FileExists(resource.path))
            return false;
        return fs::equivalent(path.data(), resource.path.c_str());
    });
    if(it != resources_.end())
    {
        return it->resourceId;
    }

    const auto resourceId = (ResourceId)resources_.size();
    resources_.emplace_back();
    Resource& newResource = resources_.back();
    newResource.path = Path(path);
    newResource.resourceId = resourceId;

    // adding a new loading job
    auto* jobSystem = GetJobSystem();
    jobSystem->AddJob(std::make_shared<LoadingResourceJob>(path, resourceId));

    return resourceId;
}

bool ResourceManager::HasLoaded(ResourceId resourceId) const
{
    return resources_[resourceId.value].hasLoaded;
}

FileBuffer* ResourceManager::GetFileBuffer(ResourceId resourceId)
{
    if(resourceId == INVALID_RESOURCE_ID)
    {
        return nullptr;
    }
    const auto& resource = resources_[resourceId.value];
    if(resource.fileIndex < 0)
    {
        return nullptr;
    }
    return &fileBuffers_[resource.fileIndex];
}

void ResourceManager::LoadingResourceJob::ExecuteImpl()
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    const auto& filesystem = core::FilesystemLocator::get();
    fileBuffer_ = filesystem.LoadFile(Path(path_));
    //add moving job to the ResourceManager
    auto* resourceManager = GetResourceManager();
    std::scoped_lock lock(resourceManager->resourceLoadMutex_);
    resourceManager->moveFileBufferJobs_.push(std::make_unique<MoveFileBufferJob>(std::move(fileBuffer_), resourceId_));
}

ResourceManager::LoadingResourceJob::LoadingResourceJob(std::string_view path, ResourceId resourceId) :
    path_(path), resourceId_(resourceId)
{

}

ResourceManager *GetResourceManager()
{
    return instance;
}

void ResourceManager::MoveFileBufferJob::ExecuteImpl()
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    auto* resourceManager = GetResourceManager();
    auto fileIndex = resourceManager->fileBuffers_.size();
    resourceManager->fileBuffers_.push_back(std::move(fileBuffer_));
    resourceManager->resources_[resourceId_.value].fileIndex = (int)fileIndex;
    resourceManager->resources_[resourceId_.value].hasLoaded = true;
}

ResourceManager::MoveFileBufferJob::MoveFileBufferJob(FileBuffer&& filebuffer, ResourceId resourceId) :
    fileBuffer_(std::move(filebuffer)), resourceId_(resourceId)
{

}
}