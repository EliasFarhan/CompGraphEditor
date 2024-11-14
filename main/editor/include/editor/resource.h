#pragma once

#include "editor_filesystem.h"

namespace editor
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
    ResourceId resourceId = INVALID_RESOURCE_ID;
    std::string path;
    std::string extension;
    fs::file_time_type lastTimeWrite;
};

class ResourceChangeInterface
{
public:
    virtual ~ResourceChangeInterface() = default;
    virtual void AddResource(const Resource& resource) = 0;
    virtual void RemoveResource(const Resource& resource) = 0;
    virtual void UpdateExistingResource(const Resource& resource) = 0;
};

}

template<>
struct std::hash<editor::ResourceId>
{
    std::size_t operator()(editor::ResourceId const& s) const noexcept
    {
        return std::hash<std::uint32_t>{}(s.value);
    }
};