#pragma once

#include "engine/filesystem.h"
#include "engine/system.h"
#include "utils/job_system.h"

namespace core
{

struct ResourceId
{
    std::uint32_t value = 0;
    constexpr bool operator==(ResourceId other) const
    {
        return value == other.value;
    }
};

constexpr ResourceId INVALID_RESOURCE_ID = {};

struct Resource
{
    Path path{};
    ResourceId resourceId = INVALID_RESOURCE_ID;
    int fileIndex = -1;
    bool hasLoaded = false;
};



class ResourceManager : public System
{
public:
    ResourceManager();
    void Begin() override;

    void Update(float dt) override;

    void End() override;

    ResourceId AddResource(std::string_view path);
    [[nodiscard]] bool HasLoaded(ResourceId resourceId) const;
    FileBuffer* GetFileBuffer(ResourceId resourceId);
protected:
    class LoadingResourceJob : public Job
    {
    public:
        LoadingResourceJob(std::string_view path, ResourceId resourceId);
    private:
        void ExecuteImpl() override;

        std::string path_;
        ResourceId resourceId_;
        FileBuffer fileBuffer_{};
    };
    class MoveFileBufferJob : public Job
    {
    public:
        MoveFileBufferJob(FileBuffer&& filebuffer, ResourceId resourceId);

    protected:
        void ExecuteImpl() override;
        FileBuffer fileBuffer_;
        ResourceId resourceId_;
    };
    std::vector<Resource> resources_;
    std::vector<FileBuffer> fileBuffers_;
    std::queue<std::unique_ptr<MoveFileBufferJob>> moveFileBufferJobs_;
    int resourceLoadQueue_ = 0;
#ifdef TRACY_ENABLE
    mutable TracyLockable (std::mutex, resourceLoadMutex_);
#else
    mutable std::mutex resourceLoadMutex_;
#endif
};

ResourceManager* GetResourceManager();

}

template<>
struct std::hash<core::ResourceId>
{
    std::size_t operator()(core::ResourceId const& s) const noexcept
    {
        return std::hash<std::uint32_t>{}(s.value);
    }
};