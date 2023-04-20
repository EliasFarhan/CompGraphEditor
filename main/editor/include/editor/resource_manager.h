#pragma once

#include <span>
#include <string>
#include <vector>
#include "resource.h"

#include "proto/renderer.pb.h"


namespace editor
{


class ResourceManager
{
public:

    template<typename T>
    using PbRepeatField = google::protobuf::RepeatedPtrField<T>;
    void CheckDataFolder(const PbRepeatField<std::string>& paths);
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