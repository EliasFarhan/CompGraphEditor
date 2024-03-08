#pragma once

#include <span>
#include <string>
#include <vector>
#include "resource.h"

#include "proto/renderer.pb.h"
#include "engine/filesystem.h"


namespace editor
{


class ResourceManager
{
public:

    template<typename T>
    using PbRepeatField = google::protobuf::RepeatedPtrField<T>;
    void CheckDataFolder(const PbRepeatField<std::string>& paths);
    [[nodiscard]] ResourceId FindResourceByPath(const core::Path &path) const;
    [[nodiscard]] const Resource* GetResource(ResourceId resource) const;
    static constexpr core::Path dataFolder = "data/";
    void RegisterResourceChange(ResourceChangeInterface* resourceChange);
    void AddResource(const core::Path &path);
    void RemoveResource(const core::Path &path, bool deleteFile=true);
    void UpdateExistingResource(const Resource& resource);
    void Clear();
private:
    static ResourceId GenerateResourceId();

    std::vector<Resource> resources_;
    std::vector<ResourceChangeInterface*> resourceChangeInterfaces_;
    void RemoveResource(const Resource& resource);
};


}